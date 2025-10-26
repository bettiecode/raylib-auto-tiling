/*
TODO:
-objects
-gui
-colors
-optimized rendering
*/

#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <rcamera.h>

#include <vector>
#include <cmath>
#include <cstdlib>
#include <fstream>

#include "constants.h"
#include "levels.cpp"

#pragma region vars
float gap;
float startX;
float startY;
float endX;
float endY;

int state = 0;

struct {
    int x;
    int y;
} mappedMouse;

struct {
    Vector2 pos = {0,0};
    float zoom = 1.0f;
} cam;

struct {
    float moveSensitivity = 1;
    float zoomSensitivity = 0.1f;
} settings;
#pragma endregion

class Popup
{
    public:
        Vector2 pos;
        int width;
        int height;
        std::string message;
        std::string input="sdsdsa";
        std::string task;
        Popup(float x, float y, int w, int h, std::string m)
        {
            pos={x,y};
            width=w;
            height=h;
            message=m;
        }
        void draw()
        {
            DrawRectangle(pos.x,pos.y,width,height,BLACK);
            DrawRectangleLines(pos.x,pos.y,width,height,WHITE);
            DrawText(message.c_str(),pos.x+8,pos.y+8,32,WHITE);
            DrawText(input.c_str(),pos.x+8,pos.y+48,16,GRAY);
        }
        void update()
        {
            
        }
};

std::vector<Popup> popups;

class Game
{
    public:
        void draw()
        {
            BeginDrawing();
                ClearBackground(DARKGRAY);
                //grid


                    int gridLineWidth = 1;

                    for(int x = 0; x < level.width+1; x++)
                    {
                        float drawX = SCREEN_WIDTH/2+(float)((x-cam.pos.x-level.width/2)*gap);
                        DrawLineEx({drawX,startY},{drawX,endY},gridLineWidth,WHITE);
                    }
                    for(int y = 0; y < level.height+1; y++)
                    {
                        float drawY = SCREEN_HEIGHT/2+(float)((y-cam.pos.y-level.height/2)*gap);
                        DrawLineEx({startX,drawY},{endX,drawY},gridLineWidth,WHITE);
                    }
                //tiles
                    for(int i = 0; i < level.textureIdxMap.size(); i++)
                    {
                        int textureIdx = level.textureIdxMap[i]-1;
                        
                        
                        int x = i % level.width - cam.pos.x;
                        int y = (i / level.width) % level.height - cam.pos.y;
                        int z = i / (level.width*level.height);

                        
                        float drawX = SCREEN_WIDTH/2+(float)((x-level.width/2)*gap);
                        float drawY = SCREEN_HEIGHT/2+(float)((y-level.height/2)*gap);
                        
                        
                        Rectangle srcRect = {0.0f,0.0f,8.0f,8.0f};
                        Rectangle destRect = {
                            drawX,
                            drawY,
                            gap,
                            gap
                        };
                        //DrawRectangleLines(drawX,drawY,gap,gap,WHITE);
                        
                        if(textureIdx < 0){continue;}
                        Color color = !(z==0) ? WHITE : DARKGRAY;
                        DrawTexturePro(level.loadedTileTextures[textureIdx],srcRect,destRect,{0.0f,0.0f},0.0f,color);
                    }
                //mouse
                    DrawRectangle(
                        SCREEN_WIDTH/2+(float)((mappedMouse.x-cam.pos.x-level.width/2)*gap),
                        SCREEN_HEIGHT/2+(float)((mappedMouse.y-cam.pos.y-level.height/2)*gap),
                        gap,gap,GLOW);
                //popups
                    for(Popup & popup : popups)
                    {
                        popup.draw();
                    }
                
                //debug
                    DrawText(std::to_string(level.materialIdxMap[mappedMouse.y*level.width+mappedMouse.x]).c_str(),0,0,16,WHITE);
                    DrawText(std::to_string(level.textureIdxMap[mappedMouse.y*level.width+mappedMouse.x]).c_str(),0,20,16,WHITE);
                    DrawText(std::to_string(mappedMouse.x).c_str(),0,40,16,WHITE);
                    DrawText(std::to_string(mappedMouse.y).c_str(),30,40,16,WHITE);
            EndDrawing();
        }
        void input()
        {
            switch(state)
            {
                case 0: //draw mode
                {
                    if(IsKeyDown(KEY_LEFT_SHIFT))
                    {
                        //camera movement x
                        cam.pos.x-=GetMouseWheelMove()*settings.moveSensitivity;
                        if (cam.pos.x < -level.width/2) cam.pos.x = -level.width/2;
                        if (cam.pos.x > level.width/2) cam.pos.x = level.width/2;
                    }
                    else if (IsKeyDown(KEY_LEFT_CONTROL))
                    {
                        //camera movement y
                        cam.pos.y-=GetMouseWheelMove()*settings.moveSensitivity;
                        if (cam.pos.y < -level.height/2) cam.pos.y = -level.height/2;
                        if (cam.pos.y > level.height/2) cam.pos.y = level.height/2;
                    }
                    else
                    {
                        //zoom
                        cam.zoom+=GetMouseWheelMove()*settings.zoomSensitivity;
                        if (cam.zoom < 0.1) cam.zoom=0.1;
                        if (cam.zoom > 5) cam.zoom=5; 
                    }
                    gap = TILE_SIZE*SCALE*cam.zoom;
                    startX = SCREEN_WIDTH/2+(float)((-cam.pos.x-level.width/2)*gap);
                    startY = SCREEN_HEIGHT/2+(float)((-cam.pos.y-level.height/2)*gap);
                    endX = SCREEN_WIDTH/2+(float)((level.width-cam.pos.x-level.width/2)*gap);
                    endY = SCREEN_HEIGHT/2+(float)((level.height-cam.pos.y-level.height/2)*gap);
                    //mouse
                    Vector2 mousePos = GetMousePosition();
                    float width = endX-startX;
                    float relativeMouseX = mousePos.x-startX;
                    mappedMouse.x = floor((relativeMouseX/width)*level.width);
                    float height = endY-startY;
                    float relativeMouseY = mousePos.y-startY;
                    mappedMouse.y = floor((relativeMouseY/height)*level.height);
                    
                    int z = IsKeyDown(KEY_LEFT_ALT);
                    if(IsKeyDown(KEY_LEFT_CONTROL))
                    {
                        //save
                        if(IsKeyPressed(KEY_S))
                        {
                            level.save("new");
                            /*
                            popups.push_back(Popup(300,300,500,200,"Save file as:"));
                            popups.shrink_to_fit();
                            state=1;
                            */
                        }
                    }
                    else if(IsKeyDown(KEY_LEFT_SHIFT))
                    {

                    }
                    else
                    {
                        if(IsKeyPressed(KEY_Q))
                        {
                        level.setXYZ(level.materialIdxMap,mappedMouse.x,mappedMouse.y,z,0);
                        }
                        if(IsKeyPressed(KEY_W))
                        {
                            level.setXYZ(level.materialIdxMap,mappedMouse.x,mappedMouse.y,z,1);
                        }
                        if(IsKeyPressed(KEY_E))
                        {
                            level.setXYZ(level.materialIdxMap,mappedMouse.x,mappedMouse.y,z,2);
                        }
                    }
                    break;
                }   
                case 1: //pause/text prompt
                if(popups.size()>0)
                {
                    for(Popup & popup : popups){popup.update();}
                    if(IsKeyPressed(KEY_X)) popups.pop_back();
                }
                else
                {
                    state=0;
                }

                break;

            }
            
        }
        void update()
        {
            level.textureIdxMap = {};
            
            
            
            for(int i = 0; i < level.materialIdxMap.size(); i++)
            {
                

                int x = i % level.width;
                int y = (i / level.width) % level.height;
                int z = i / (level.width*level.height);

                int current = level.getXYZ(level.materialIdxMap,x,y,z);
                
                if (current == 0)
                {
                    level.textureIdxMap.push_back(0); // end 0: tile is empty, return 0
                    continue;  
                } 
                
                TileMat mat = level.materials[current-1];

                if (mat.doesAutoTile==false)
                {
                    level.textureIdxMap.push_back(mat.firstIdx+1); // end 1: tile does not auto tile, return firstIdx
                    continue;
                }
                
                int up = y > 0 ? level.getXYZ(level.materialIdxMap,x,y-1,z): 0;
                int down = y < level.height-1 ? level.getXYZ(level.materialIdxMap,x,y+1,z) : 0;
                int left = x > 0 ? level.getXYZ(level.materialIdxMap,x-1,y,z) : 0;
                int right = x < level.width-1 ? level.getXYZ(level.materialIdxMap,x+1,y,z) : 0;

                int val = 15;
                if(up==current){val-=8;}
                if(right==current){val-=4;}
                if(down==current){val-=2;}
                if(left==current){val-=1;}

                level.textureIdxMap.push_back(mat.firstIdx+val+1); // end 2: tile does auto tile, return firstIdx+val

            }
            level.textureIdxMap.shrink_to_fit();
        }
};


Game game;


int main()
{


    InitWindow(SCREEN_WIDTH,SCREEN_HEIGHT,"Raylib Auto-Tiling (16-tileset) C++");
    SetTargetFPS(60);
    ShowCursor();
    
    
    level.load("test");

    while (WindowShouldClose() == false)
    {
        //input
        game.input();
        //update
        game.update();
        //render
        game.draw();
    }
    level.save("test");
    level.unload();
    CloseWindow();

    #pragma region debug
    //std::cout << level.materialIdxMap.size() << " - " << level.height*level.width << "\n";
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
    #pragma endregion
    return 0;
}