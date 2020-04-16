#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#include <iostream>
#include <stdlib.h>
#include <fstream>

#include <utils.h>
#include "Scene.h"
#include "Client.h"

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define N_CHAR 1024UL

/**
 * Scène à dessiner
 * NB: son constructeur doit être appelé après avoir initialisé OpenGL
 **/
Scene *scene = nullptr;

/**
 * Callback pour GLFW : prendre en compte la taille de la vue OpenGL
 **/
static void onSurfaceChanged(GLFWwindow *window, int width, int height)
{
    if (scene == nullptr)
        return;
    scene->onSurfaceChanged(width, height);
}

/**
 * Callback pour GLFW : redessiner la vue OpenGL
 **/
static void onDrawRequest(GLFWwindow *window)
{
    if (scene == nullptr)
        return;
    Utils::UpdateTime();
    scene->onDrawFrame();
    static bool premiere = true;
    if (premiere)
    {
        // copie écran automatique
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        Utils::ScreenShotPPM("image.ppm", width, height);
        premiere = false;
    }

    // afficher le back buffer
    glfwSwapBuffers(window);
}

static void onMouseButton(GLFWwindow *window, int button, int action, int mods)
{
    if (scene == nullptr)
        return;
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    if (action == GLFW_PRESS)
    {
        scene->onMouseDown(button, x, y);
    }
    else
    {
        scene->onMouseUp(button, x, y);
    }
}

static void onMouseMove(GLFWwindow *window, double x, double y)
{
    if (scene == nullptr)
        return;
    scene->onMouseMove(x, y);
}

static void onKeyboard(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE)
        return;
    if (scene == nullptr)
        return;
    scene->onKeyDown(key);
}

void onExit()
{
    // libération des ressources demandées par la scène
    if (scene != nullptr)
        delete scene;
    scene = nullptr;

    // terminaison de GLFW
    glfwTerminate();

    // libération des ressources openal
    alutExit();

    // retour à la ligne final
    std::cout << std::endl;
}

/** appelée en cas d'erreur */
void error_callback(int error, const char *description)
{
    std::cerr << "GLFW error : " << description << std::endl;
}

/** point d'entrée du programme **/
int main(int argc, char **argv)
{
    bool isComplete = false;
    std::thread connection_dealer = std::thread(deal_with_socket);
    while (!isComplete)
    {
    }
    connection_dealer.detach();
    server_dealer.detach();

    return EXIT_SUCCESS;
}

void deal_with_socket()
{
    struct sockaddr_in serv_addr;
    int new_socket;

    if ((new_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "Socket creation error" << std::endl;
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, SERV_ADDR, &serv_addr.sin_addr) <= 0)
    {
        std::cerr << "Invalid address / Address not supported " << std::endl;
        exit(EXIT_FAILURE);
    }

    if (connect(new_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "Connection Failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::thread server_dealer = std::thread(deal_with_server, new_socket);
    std::thread game_dealer = std::thread(deal_with_game, new_socket);
    while (!isComplete)
    {
    }
}

void deal_with_server(int new_socket) {
        int client_id, end_conf = -1;
    size_t valread;
    char buffer[N_CHAR];

    // init connexion with server by sending CONNEXION message and get the client_id
    std::string msg = "CONNECTION";
    send(new_socket, msg.c_str(), msg.length(), 0);
    std::cout << msg << " message sent to the server" << std::endl;
    valread = read(new_socket, buffer, N_CHAR);
    std::string message(buffer);
    std::cout << "Client id : " << message << std::endl;
    client_id = atoi(message.c_str());

    // get duck configuration line by line
    msg = "CONFIGURATION";
    send(new_socket, msg.c_str(), msg.length(), 0);
    std::ofstream file("duckconfig.txt");
    do
    {
        memset(buffer, 0, sizeof(buffer));
        valread = read(new_socket, buffer, N_CHAR);
        message = buffer;
        if (message.compare(0, sizeof("END_CONFIGURATION"), "END_CONFIGURATION") != 0)
        {
            file << message << std::endl;
        }
    } while (message.compare(0, sizeof("END_CONFIGURATION"), "END_CONFIGURATION") != 0);
    file.close();
}

void deal_with_game(int new_socket)
{
    // initialisation de GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }
    glfwSetErrorCallback(error_callback);

    // caractéristiques de la fenêtre à ouvrir
    //glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing

    // initialisation de la fenêtre
    GLFWwindow *window = glfwCreateWindow(640, 480, "Livre OpenGL", NULL, NULL);
    if (window == nullptr)
    {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowPos(window, 200, 200);
    glfwSetWindowTitle(window, "Cameras - TurnTable");

    // initialisation de glew
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cerr << "Unable to initialize Glew : " << glewGetErrorString(err) << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // pour spécifier ce qu'il faut impérativement faire à la sortie
    atexit(onExit);

    // initialisation de la bibliothèque de gestion du son
    alutInit(0, NULL);
    alGetError();

    // position de la caméra qui écoute
    alListener3f(AL_POSITION, 0, 0, 0);
    alListener3f(AL_VELOCITY, 0, 0, 0);

    // création de la scène => création des objets...
    scene = new Scene("duckconfig.txt", new_socket);
    //debugGLFatal("new Scene()");

    // enregistrement des fonctions callbacks
    glfwSetFramebufferSizeCallback(window, onSurfaceChanged);
    glfwSetCursorPosCallback(window, onMouseMove);
    glfwSetMouseButtonCallback(window, onMouseButton);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetKeyCallback(window, onKeyboard);

    // affichage du mode d'emploi
    std::cout << "Usage:" << std::endl;
    std::cout << "Left button to rotate object" << std::endl;
    std::cout << "Q,D (axis x) A,W (axis y) Z,S (axis z) keys to move" << std::endl;

    // boucle principale
    onSurfaceChanged(window, 640, 480);
    do
    {
        // dessiner
        onDrawRequest(window);
        // attendre les événements
        glfwPollEvents();
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(window) || isComplete);

    return exit(EXIT_SUCCESS);
}
