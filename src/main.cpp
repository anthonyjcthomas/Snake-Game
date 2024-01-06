#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>

using namespace std;

// Game settings and colors
static bool allowMove = false;
Color green = {173, 204, 96, 255}; // Light green color
Color darkGreen = {43, 51, 24, 255}; // Dark green color

// Game board settings
int cellSize = 30; // Size of each cell in the game grid
int cellCount = 25; // Number of cells in the game grid
int offset = 75; // Offset for drawing

double lastUpdateTime = 0;

// Function to check if an element is present in a deque
bool ElementInDeque(Vector2 element, deque<Vector2> deque)
{
    for (unsigned int i = 0; i < deque.size(); i++)
    {
        if (Vector2Equals(deque[i], element))
        {
            return true;
        }
    }
    return false;
}

// Function to handle timed events
bool EventTriggered(double interval)
{
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval)
    {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

// Snake class
class Snake
{
public:
    deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    Vector2 direction = {1, 0};
    bool addSegment = false;

    // Function to draw the snake
    void Draw()
    {
        for (unsigned int i = 0; i < body.size(); i++)
        {
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = Rectangle{offset + x * cellSize, offset + y * cellSize, (float)cellSize, (float)cellSize};
            DrawRectangleRounded(segment, 0.5, 6, darkGreen);
        }
    }

    // Function to update the snake's position
    void Update()
    {
        body.push_front(Vector2Add(body[0], direction));
        if (addSegment == true)
        {
            addSegment = false;
        }
        else
        {
            body.pop_back();
        }
    }

    // Function to reset the snake to initial state
    void Reset()
    {
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        direction = {1, 0};
    }
};

// Food class
class Food
{
public:
    Vector2 position;
    Texture2D texture;

    // Constructor to initialize food
    Food(deque<Vector2> snakeBody)
    {
        Image image = LoadImage("Graphics/food.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = GenerateRandomPos(snakeBody);
    }

    // Destructor to unload texture
    ~Food()
    {
        UnloadTexture(texture);
    }

    // Function to draw food
    void Draw()
    {
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }

    // Function to generate a random cell
    Vector2 GenerateRandomCell()
    {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }

    // Function to generate a random position for food
    Vector2 GenerateRandomPos(deque<Vector2> snakeBody)
    {
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody))
        {
            position = GenerateRandomCell();
        }
        return position;
    }
};

// Game class
class Game
{
public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    bool running = true;
    int score = 0;
    Sound eatSound;
    Sound wallSound;

    // Constructor to initialize game
    Game()
    {
        InitAudioDevice();
        eatSound = LoadSound("Sounds/eat.mp3");
        wallSound = LoadSound("Sounds/wall.mp3");
    }

    // Destructor to unload sounds
    ~Game()
    {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }

    // Function to draw game elements
    void Draw()
    {
        food.Draw();
        snake.Draw();
    }

    // Function to update game state
    void Update()
    {
        if (running)
        {
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithTail();
        }
    }

    // Function to check collision with food
    void CheckCollisionWithFood()
    {
        if (Vector2Equals(snake.body[0], food.position))
        {
            food.position = food.GenerateRandomPos(snake.body);
            snake.addSegment = true;
            score++;
            PlaySound(eatSound);
        }
    }

    // Function to check collision with edges
    void CheckCollisionWithEdges()
    {
        if (snake.body[0].x == cellCount || snake.body[0].x == -1)
        {
            GameOver();
        }
        if (snake.body[0].y == cellCount || snake.body[0].y == -1)
        {
            GameOver();
        }
    }

    // Function to handle game over
    void GameOver()
    {
        snake.Reset();
        food.position = food.GenerateRandomPos(snake.body);
        running = false;
        score = 0;
        PlaySound(wallSound);
    }

    // Function to check collision with tail
    void CheckCollisionWithTail()
    {
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if (ElementInDeque(snake.body[0], headlessBody))
        {
            GameOver();
        }
    }
};

// Main function
int main()
{
    cout << "Starting the game..." << endl;
    // Initialize window
    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Retro Snake");
    SetTargetFPS(60);

    Game game = Game();

    // Game loop
    while (WindowShouldClose() == false)
    {
        BeginDrawing();

        // Update game every 0.2 seconds
        if (EventTriggered(0.2))
        {
            allowMove = true;
            game.Update();
        }

        // Handle input
        if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1 && allowMove)
        {
            game.snake.direction = {0, -1};
            game.running = true;
            allowMove = false;
        }
        if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1 && allowMove)
        {
            game.snake.direction = {0, 1};
            game.running = true;
            allowMove = false;
        }
        if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1 && allowMove)
        {
            game.snake.direction = {-1, 0};
            game.running = true;
            allowMove = false;
        }
        if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1 && allowMove)
        {
            game.snake.direction = {1, 0};
            game.running = true;
            allowMove = false;
        }

        // Draw game elements
        ClearBackground(green);
        DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10}, 5, darkGreen);
        DrawText("Retro Snake", offset - 5, 20, 40, darkGreen);
        DrawText(TextFormat("%i", game.score), offset - 5, offset + cellSize * cellCount + 10, 40, darkGreen);
        game.Draw();

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
