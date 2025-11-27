#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <cmath> // za pi
#include <algorithm> // za max()
#include <iostream>
#include "Util.h"


#define NUM_SLICES 40

int endProgram(std::string message) {
    std::cout << message << std::endl;
    glfwTerminate();
    return -1;
}

float goldX = 0.0f, goldY = -0.5f;   // goldFish pozicija
float blowX = 0.5f, blowY = 0.0f;    // blowFish pozicija
float moveSpeed = 0.005f;

// Dimenzije ekrana su izdvojene kao globalne promenljive zbog zadatka 6
int screenWidth = 1000;
int screenHeight = 800;

float initialJumpTime = -1.0f;

void preprocessTexture(unsigned& texture, const char* filepath) {
    texture = loadImageToTexture(filepath); // Učitavanje teksture
    glBindTexture(GL_TEXTURE_2D, texture); // Vezujemo se za teksturu kako bismo je podesili

    // Generisanje mipmapa - predefinisani različiti formati za lakše skaliranje po potrebi (npr. da postoji 32 x 32 verzija slike, ali i 16 x 16, 256 x 256...)
    glGenerateMipmap(GL_TEXTURE_2D);

    // Podešavanje strategija za wrap-ovanje - šta da radi kada se dimenzije teksture i poligona ne poklapaju
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // S - tekseli po x-osi
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // T - tekseli po y-osi

    // Podešavanje algoritma za smanjivanje i povećavanje rezolucije: nearest - bira najbliži piksel, linear - usrednjava okolne piksele
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void formVAOs(float* verticesRect, size_t rectSize, unsigned int& VAOrect) {
    // formiranje VAO-ova je izdvojeno u posebnu funkciju radi čitljivijeg koda u main funkciji

    // Podsetnik za atribute:
    /*
        Jedan VAO se vezuje za jedan deo celokupnog skupa verteksa na sceni.
        Na primer, dobra praksa je da se jedan VAO vezuje za jedan VBO koji se vezuje za jedan objekat, odnosno niz temena koja opisuju objekat.

        VAO je pomoćna struktura koja opisuje kako se podaci u nizu objekta interpretiraju.
        U render-petlji, za crtanje određenog objekta, naredbom glBindVertexArray(nekiVAO) se određuje koji se objekat crta.

        Potrebno je definisati koje atribute svako teme u nizu sadrži, npr. pozicija na lokaciji 0 i boja na lokaciji 1.

        Ova konfiguracija je specifična za naš primer na vežbama i može se menjati za različite potrebe u projektu.


        Atribut se opisuje metodom glVertexAttribPointer(). Argumenti su redom:
            index - identifikacioni kod atributa, u verteks šejderu je povezan preko svojstva location (location = 0 u šejderu -> indeks tog atributa treba staviti isto 0 u ovoj naredbi)
            size - broj vrednosti unutar atributa (npr. za poziciju su x i y, odnosno 2 vrednosti; za boju r, g i b, odnosno 3 vrednosti)
            type - tip vrednosti
            normalized - da li je potrebno mapirati na odgovarajući opseg (mi poziciju već inicijalizujemo na opseg (-1, 1), a boju (0, 1), tako da nam nije potrebno)
            stride - koliko elemenata u nizu treba preskočiti da bi se došlo od datog atributa u jednom verteksu do istog tog atributa u sledećem verteksu
            pointer - koliko elemenata u nizu treba preskočiti od početka niza da bi se došlo do prvog pojavljivanja datog atributa
    */
    // Četvorougao

    unsigned int VBOrect;
    glGenVertexArrays(1, &VAOrect);
    glGenBuffers(1, &VBOrect);

    glBindVertexArray(VAOrect);
    glBindBuffer(GL_ARRAY_BUFFER, VBOrect);
    glBufferData(GL_ARRAY_BUFFER, rectSize, verticesRect, GL_STATIC_DRAW);

    // Atribut 0 (pozicija):
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Atribut 1 (boja):
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void drawRect(unsigned int shader, unsigned int VAO, float x, float y)
{
    glUseProgram(shader);
    glUniform1f(glGetUniformLocation(shader, "uX"), x);
    glUniform1f(glGetUniformLocation(shader, "uY"), y);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

int main()
{
    // Inicijalizacija GLFW i postavljanje na verziju 3 sa programabilnim pajplajnom
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Formiranje prozora za prikaz sa datim dimenzijama i naslovom
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Vezba 3", NULL, NULL);
    if (window == NULL) return endProgram("Prozor nije uspeo da se kreira.");
    glfwMakeContextCurrent(window);


    // Inicijalizacija GLEW
    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    // Potrebno naglasiti da program koristi alfa kanal za providnost
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    unsigned int rectShader = createShader("rect.vert", "rect.frag");
    unsigned int colorShader = createShader("color.vert", "color.frag");

    float goldFish[] = {
         -0.2f, 0.2f, 0.0f, 0.0f, 1.0f, // gornje levo teme
         -0.2f, -0.2f, 0.0f, 1.0f, 0.0f, // donje levo teme
         0.2f, -0.2f, 1.0f, 0.0f, 0.0f, // donje desno teme
         0.2f, 0.2f, 0.0f, 1.0f, 1.0f, // gornje desno teme
    };

    float blowFish[] = {
         -0.1f, 0.1f, 0.0f, 0.0f, 1.0f, // gornje levo teme
         -0.1f, -0.1f, 0.0f, 1.0f, 0.0f, // donje levo teme
         0.1f, -0.1f, 1.0f, 0.0f, 0.0f, // donje desno teme
         0.1f, 0.1f, 0.0f, 1.0f, 1.0f, // gornje desno teme
    };

    unsigned int VAOGoldFish;
    unsigned int VAOBlowFish;

    formVAOs(goldFish, sizeof(goldFish), VAOGoldFish);
    formVAOs(blowFish, sizeof(blowFish), VAOBlowFish);

    glClearColor(0.5f, 0.6f, 1.0f, 1.0f); // Postavljanje boje pozadine

    while (!glfwWindowShouldClose(window))
    {
        // Procesovanje inputa sa tastature je izolovan od crtanja
        // Ukoliko postoji puno tastera sa različitim funkcijama, moguće je razdvojiti processInput() kao posebnu funkciju
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) goldX -= moveSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) goldX += moveSpeed;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) goldY += moveSpeed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) goldY -= moveSpeed;

        // BLOW FISH – pomeraj na strelice
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  blowX -= moveSpeed;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) blowX += moveSpeed;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    blowY += moveSpeed;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  blowY -= moveSpeed;

        glClear(GL_COLOR_BUFFER_BIT); // Bojenje pozadine, potrebno kako pomerajući objekti ne bi ostavljali otisak

        drawRect(rectShader, VAOGoldFish, goldX, goldY);
        drawRect(rectShader, VAOBlowFish, blowX, blowY);

        glfwSwapBuffers(window); // Zamena bafera - prednji i zadnji bafer se menjaju kao štafeta; dok jedan procesuje, drugi se prikazuje.
        glfwPollEvents(); // Sinhronizacija pristiglih događaja
    }

    glDeleteProgram(rectShader);
    glDeleteProgram(colorShader);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}