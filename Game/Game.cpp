#include "ErrorHandler.h"
#include "Game.h"
#include "SDL_image.h"

Game::Game(const int numTilesWidth, const int numTilesHeight, const char* title, bool fullscreen)
    : screenScale(getScreenScale(fullscreen))
    , display(numTilesWidth, numTilesHeight)
{
    int flags = SDL_WINDOW_ALLOW_HIGHDPI;

    if (fullscreen)
    {
        flags = flags|SDL_WINDOW_FULLSCREEN;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        throw Exception(SDL_GetError());
    }

    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0, &displayMode) != 0)
    {
        throw Exception(SDL_GetError());
    }
    display.setDisplaySize(displayMode.w, displayMode.h, screenScale, false);

    window = SDL_CreateWindow(title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        display.getGameWidth(),
        display.getGameHeight(),
        flags);
    if (window == nullptr)
    {
        throw Exception(SDL_GetError());
    }

    std::string iconPath = basePath + "assets/Icon1024x1024.png";
    SDL_Surface* icon = IMG_Load(iconPath.c_str());
    if (icon == nullptr)
    {
        throw Exception(SDL_GetError());
    }
    SDL_SetWindowIcon(window, icon);
    SDL_FreeSurface(icon);

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == nullptr)
    {
        throw Exception(SDL_GetError());
    }

    int pixelWidth;
    int pixelHeight;
    SDL_GetRendererOutputSize(renderer, &pixelWidth, &pixelHeight);
    display.setDisplaySize(pixelWidth, pixelHeight, 1.0f, true);
    renderRect.x = (display.getScreenWidth()-display.getGameWidth())/2;
    renderRect.y = (display.getScreenHeight()-display.getGameHeight())/2;
    renderRect.w = display.getGameWidth();
    renderRect.h = display.getGameHeight();

    if (TTF_Init() != 0)
    {
        throw Exception(SDL_GetError());
    }

    std::string fontPath = basePath + "assets/OpenSans-Regular.ttf";
    font = TTF_OpenFont(fontPath.c_str(), 100);
    if (font == nullptr)
    {
        throw Exception(SDL_GetError());
    }
}

Game::~Game()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    SDL_Quit();
}

const float Game::getScreenScale(bool fullscreen)
{
    if (fullscreen)
    {
        return 1.0f;
    }
    else
    {
        return 0.8f;
    }
}

float Game::getPixelsToPointsScaleFactor()
{
    int fontSize = 100;
    TTF_SetFontSize(font, fontSize);

    int height = TTF_FontHeight(font);
    if (height <= 0)
    {
        throw Exception("Font height is " + std::to_string(height));
    }

    return static_cast<float>(fontSize) / static_cast<float>(height);
}

void Game::text(const char * text, int fontSizeHeightPercent, SDL_Color& fontColor, int x, int y, bool centered)
{
    float scale = getPixelsToPointsScaleFactor();
    int heightPixels = display.heightPercentToPixels(fontSizeHeightPercent);
    int fontSize = static_cast<int>(heightPixels * scale);

    TTF_SetFontSize(font, fontSize);

    SDL_Surface* surf = TTF_RenderText_Blended(font, text, fontColor);

    SDL_Texture* labelTexture = SDL_CreateTextureFromSurface(renderer, surf);

    int textureWidth = surf->w;
    int textureHeight = surf->h;
    SDL_FreeSurface(surf);

    if (centered)
    {
        x = (display.getGameWidth() - textureWidth) / 2 - 3;
    }

    SDL_Rect renderQuad = { x, y, textureWidth, textureHeight };

    SDL_RenderCopyEx(renderer, labelTexture, nullptr, &renderQuad, 0.0, nullptr, SDL_FLIP_NONE);
    SDL_DestroyTexture(labelTexture);
}

void Game::renderSetViewport()
{
    if (SDL_RenderSetViewport(renderer, &renderRect) != 0)
    {
        throw Exception(SDL_GetError());
    }
}

void Game::setRenderDrawColor(const SDL_Color& color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void Game::renderClear()
{
    SDL_RenderClear(renderer);
}

void Game::renderPresent()
{
    SDL_RenderPresent(renderer);
    calculateFPS();
}

void Game::calculateFPS() {
    static std::chrono::time_point<std::chrono::steady_clock> oldTime = std::chrono::high_resolution_clock::now();
    static int numFrames; numFrames++;

    if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - oldTime) >= std::chrono::seconds{ 1 }) {
        oldTime = std::chrono::high_resolution_clock::now();
        fps = numFrames;
        numFrames = 0;
    }
}

void Game::renderFillRect(const SDL_Rect& rect, const SDL_Color& color)
{
    setRenderDrawColor(color);
    SDL_RenderFillRect(renderer, &rect);
}

void Game::playMusic(const std::string& musicPath)
{
    std::string fullMusicPath = basePath + musicPath;

    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        throw Exception(Mix_GetError());
    }
    music = Mix_LoadMUS(fullMusicPath.c_str());
    if(music == nullptr)
    {
        throw Exception(Mix_GetError());
    }
    Mix_PlayMusic(music, -1);
}

void Game::stopMusic()
{
    Mix_FadeOutMusic(100);
    Mix_FreeMusic(music);
    music = nullptr;
}
