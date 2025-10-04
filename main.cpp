#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <rcamera.h>
#include <vector>
#include <cmath>
#include <cstdlib>

struct LevelData
{
    
        int width;
        int height;
        
        int materialTextureSourceCount;
        std::string materialTextureSourceFilenames[16];

        std::string encodedTilemap;



};

LevelData levels[] = 
{
    {
        32,
        18,
        
        1,
        {"snow/ice"},

        "1x32(1);16x1(1)30(0)1(1);1x32(1);"
    }

};

std::string intToBits4(int x)
{
    int n = x;
    std::string result = "0000";
    for(int i = 0; i < 4; i++)
    {
       if(n >= pow(2,3-i))
       {
            result[i]='1';
            n-=pow(2,3-i);
       }
    }
    return result;
};

typedef std::vector<std::vector<int>> intMatrix;

const int TILE_SIZE = 8;
const int SCALE = 6;

struct {
    int width;
    int height;
    std::vector<int> materialIdxMap;
    std::vector<int> textureIdxMap;
} level;

std::vector<Texture> loadedTileTextures;

class Game
{
    public:
        void draw()
        {
            BeginDrawing();
                ClearBackground(DARKGRAY);
                for(int i = 0; i < level.textureIdxMap.size(); i++)
                {
                    int textureIdx = level.textureIdxMap[i];
                    if(textureIdx < 0){continue;}
                    
                    int x = i % level.width;
                    int y = i / level.width;
                    Rectangle srcRect = {0.0f,0.0f,8.0f,8.0f};
                    Rectangle destRect = {(float)(x*TILE_SIZE*SCALE),(float)(y*TILE_SIZE*SCALE),(float)TILE_SIZE*SCALE,TILE_SIZE*SCALE};
                    
                    DrawTexturePro(loadedTileTextures[textureIdx],srcRect,destRect,{0.0f,0.0f},0.0f,WHITE);
                }
            EndDrawing();
        }
};

void loadLevel(int idx)
{
    //maps
    level.width=levels[idx].width;
    level.height=levels[idx].height;

    //decode
    level.materialIdxMap = {};

    std::string word = "";
    
    int rowMultiplier = 0;

    std::vector<int> multipliers;
    std::vector<int> materials;

    std::cout << levels[idx].encodedTilemap << "\n";

    for(std::string::iterator it = levels[idx].encodedTilemap.begin(); it != levels[idx].encodedTilemap.end(); ++it)
    {
        switch (*it)
        {
            case 'x':
                rowMultiplier = std::stoi(word);
                word = "";
                break;
            case '(':
                multipliers.push_back(std::stoi(word));
                word = "";
                break;
            case ')':
                materials.push_back(std::stoi(word));
                word = "";
                break;
            case ';':
                for(int i = 0; i < rowMultiplier; i++)
                {
                    //std::cout << "\n" << "i" << i << "\n";
                    for(int j = 0; j < multipliers.size(); j++)
                    {
                        //std::cout << "\n" << "j" << j << "\n";
                        for(int k = 0; k < multipliers[j]; k++)
                        {
                            //std::cout << "\n" << "k" << k << "\n";
                            level.materialIdxMap.push_back(materials[j]);

                        }
                    }
                    //std::cout << level.materialIdxMap.size() << "\n";
                }
                word = "";
                rowMultiplier = 0;
                multipliers = {};
                materials = {};
                break;
            default:    
                word+=*it;
                break;
        }
        //std::cout << *it << " ";
    }
    level.materialIdxMap.shrink_to_fit();
    for(int i = 0; i < level.materialIdxMap.size(); i++)
    {
        level.materialIdxMap[i] = rand() % 2;
    }
    
    //auto-tiling
    for(int i = 0; i < level.materialIdxMap.size(); i++)
    {
        int x = i % level.width;
        int y = i / level.width;
        
        int current = level.materialIdxMap[y*level.width+x];
        int up = y > 0 ? level.materialIdxMap[(y-1)*level.width+x] : 0;
        int down = y < level.height-1 ? level.materialIdxMap[(y+1)*level.width+x] : 0;
        int left = x > 0 ? level.materialIdxMap[y*level.width+x-1] : 0;
        int right = x < level.width-1 ? level.materialIdxMap[y*level.width+x+1] : 0;

        int val = 15;
        if(up==current){val-=8;}
        if(right==current){val-=4;}
        if(down==current){val-=2;}
        if(left==current){val-=1;}

        level.textureIdxMap.push_back((current-1)*16+val);

    }
    level.textureIdxMap.shrink_to_fit();
    //level.textureIdxMap=level.materialIdxMap;
    //textures
    for(int i = 0; i < levels[idx].materialTextureSourceCount; i++)
    {
        for(int j = 0; j < 16; j++)
        {
            std::string filename = "tilesets/"+levels[idx].materialTextureSourceFilenames[i]+'-'+intToBits4(j)+".png";
            Image image = LoadImage(filename.c_str()); 
            loadedTileTextures.push_back(LoadTextureFromImage(image));
            UnloadImage(image);
            std::cout << filename << std::endl;
        }
    }
    loadedTileTextures.shrink_to_fit();
}

void unloadLevel()
{
    for(Texture texture : loadedTileTextures)
    {
        UnloadTexture(texture);
    }
}

int main()
{
    InitWindow(32*TILE_SIZE*SCALE,18*TILE_SIZE*SCALE,"Raylib Auto-Tiling (16-tileset) C++");
    SetTargetFPS(60);
    
    Game game;
    
    loadLevel(0);

    while (WindowShouldClose() == false)
    {
        game.draw();
    }
    
    unloadLevel();
    CloseWindow();

    std::cout << level.materialIdxMap.size() << " - " << level.height*level.width << "\n";
    for(int y = 0; y < level.height; y++)
    {
        for(int x = 0; x < level.width; x++)
        {
            std::cout << level.materialIdxMap[y*level.width+x];
        }
        std::cout << " " << y << "\n";
    }
    
    /*
    std::cout << level.textureIdxMap.size() << "\n";
    std::cout << loadedTileTextures.size() << "\n";
    std::cout << loadedTileTextures.capacity() << "\n";
    std::cout << level.materialMap.size() << "\n";
    std::cout << level.materialMap.capacity() << "\n";
    std::cout << level.textureIdxMap.size() << "\n";
    std::cout << level.textureIdxMap.capacity() << "\n";
    */
    return 0;
}