#include "header\playerfile.hpp"

using namespace std;

int main()
{
    string pfileName;
    cout << "    ______         ____                       ____                   "  << endl <<  "   / ____/___     / __ \\____  ____  ____ _   / __ \\____  ____  ____ _"  << endl <<   "  / / __/ __ \\   / / / / __ \\/ __ \\/ __ `/  / / / / __ \\/ __ \\/ __ `/"  << endl <<  " / /_/ / /_/ /  / /_/ / /_/ / / / / /_/ /  / /_/ / /_/ / / / / /_/ / "  << endl <<  " \\____/\\____/  /_____/\\____/_/ /_/\\__, /  /_____/\\____/_/_/_/\\__, /  "  << endl <<  "                    \\ \\/ /___  __/____/___________  / / __/ /____/   "  << endl <<  "                     \\  / __ \\/ / / / ___/ ___/ _ \\/ / /_            "  << endl <<  "                     / / /_/ / /_/ / /  (__  )  __/ / __/            "  << endl <<  "                    /_/\\____/\\__,_/_/  /____/\\___/_/_/               "  << endl
    << endl << "* v2.0 - snox - 08/2023 " << endl << endl;
    cout << "Enter character name without file extension (case sensitive): "; cin >> pfileName; cout << endl;
    PlayerFile player(pfileName);
    player.readHeader();
    player.createDir();
    player.readImages();
    player.readPallettes();
    player.createSounds();
    player.closePlayerFile();
    player.createImages();
    cout << endl << "Press any key to close." << endl;
    system("pause >nul");
    return 0;
}