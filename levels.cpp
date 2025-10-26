#include <vector>
#include <cmath>
#include <iostream>
#include <raylib.h>
#include <fstream>

class TileMat
{
    public:
        bool doesAutoTile;
        std::string textureSource;
        int firstIdx;
    TileMat(bool x, std::string y)
    {
        doesAutoTile = x;
        textureSource = y;
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

Color hexToColor(std::string code)
{
    unsigned char red = 255;
    unsigned char blue = 255;
    unsigned char green = 255;
    unsigned char alpha = 255;

    return Color{red,green,blue,alpha};

}
/*
std::string colorToHex(Color color)
{
    
}
*/
class Level
{
    public:
    
    int width;
    int height;
    int depth;

    //std::vector<Color> layerColors;

    std::vector<int> materialIdxMap;
    std::vector<int> textureIdxMap;

    std::vector<TileMat> materials;
    std::vector<Texture> loadedTileTextures;

    int getXYZ(std::vector<int> &vector, int x, int y, int z)
    {
        return vector[z*width*height+y*width+x];
    }
    void setXYZ(std::vector<int> &vector, int x, int y, int z, int val)
    {
        vector[z*width*height+y*width+x]=val;
    }

    void load(std::string levelFilename)
    {
    //decode
    std::ifstream file("saves/"+levelFilename+".dat");
    std::string line;
    
        #pragma region width height depth
        std::getline(file,line);
        std::string word;
        //std::cout << line;
        for(char ch : line)
        {
            switch(ch)
            {
                case 'x':
                    width = std::stoi(word);
                    word="";
                    break;
                case 'y':
                    height = std::stoi(word);
                    word="";
                    break;
                case 'z':
                    depth = std::stoi(word);
                    word="";
                    break;
                default:
                    word+=ch;
                    break;
            }
        }
        #pragma endregion
        /*#pragma region colors
        std::getline(file,line);
        word="";
        //std::cout << line;
        for(char ch : line)
        {
            switch(ch)
            {
                case ';':
                    layerColors.push_back(hexToColor(word));
                    word="";
                    break;
                default:
                    word+=ch;
                    break;
            }
        }
        layerColors.shrink_to_fit();
        #pragma endregion*/
        #pragma region textures and materials
        std::getline(file,line);
        bool doesAutoTile;
        word = "";
        //std::cout << line;
        for(char ch : line)
        {
            switch(ch)
            {
                case ':':
                    doesAutoTile = word=="1" ? true : false; 
                    word="";
                    break;
                case ';':
                    materials.push_back(TileMat(doesAutoTile,word));
                    word="";
                    break;
                default:
                    word+=ch;
                    break;


            }
        }
        materials.shrink_to_fit();
        int i = 0;
        for(TileMat &mat : materials)
        {
            mat.firstIdx=i;
            if (mat.doesAutoTile)
            {
                for(int j = 0; j < 16; j++)
                        {
                            std::string tileSourceFilename = "tilesets/"+mat.textureSource+'-'+intToBits4(j)+".png";
                            Image image = LoadImage(tileSourceFilename.c_str()); 
                            loadedTileTextures.push_back(LoadTextureFromImage(image));
                            UnloadImage(image);
                            std::cout << tileSourceFilename << std::endl;
                            i++;
                        }
            }else
            {
                std::string tileSourceFilename = "tilesets/"+mat.textureSource+".png";
                Image image = LoadImage(tileSourceFilename.c_str()); 
                loadedTileTextures.push_back(LoadTextureFromImage(image));
                UnloadImage(image);
                std::cout << tileSourceFilename << std::endl;
                i++;                
            }
        }
        loadedTileTextures.shrink_to_fit();
        #pragma endregion
        #pragma region material map
        std::getline(file,line);
        word="";
        for(char ch : line)
        {
            switch(ch)
            {
                case ';':
                    materialIdxMap.push_back(std::stoi(word));
                    word="";
                    break;
                case 'e':
                    for(int i = 0; i < height*width*depth; i++){materialIdxMap.push_back(0);}
                    break;
                case 'f':
                    for(int i = 0; i < height*width*depth; i++){materialIdxMap.push_back(1);}
                    break;
                case 'g':
                    for(int i = 0; i < height*width*depth; i++){materialIdxMap.push_back(2);}
                    break;
                default:
                    word+=ch;
                    break;
            }
        }
        materialIdxMap.shrink_to_fit();
        #pragma endregion
    

    file.close();
}
    void unload()
    {
        for(Texture & texture : loadedTileTextures)
        {
            UnloadTexture(texture);
        }
        materials={};
        materialIdxMap={};
        textureIdxMap={};
        loadedTileTextures={};
        //layerColors={};
    }

    void save(std::string filename)
    {
        printf("saving to file\n");
        std::ofstream file("saves/"+filename+".dat");
        file << width << 'x' << height << 'y' << depth << "z\n";
        //file << "0;0;\n";
        for(TileMat mat : materials)
        {
            file << mat.doesAutoTile << ":" << mat.textureSource << ";";
        }
        file << "\n";
        for(int item : materialIdxMap)
        {
            file << item << ";";
        }
        file.close();
    }
};

Level level;





