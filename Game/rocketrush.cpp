#include <ncurses/ncurses.h> 
#include <cstdlib> 
#include <ctime>  
#include <vector> 
#include <string>   
#include <fstream>      
#include <sstream>     
#include <algorithm>  
#include <windows.h>       
#include <mmsystem.h> 

using namespace std;

const int limitMeteor = 10;
const int jedaBintang = 120;
const int kelipatanSkor = 200;
const int kecepatanNormal = 50;
const int fps = 20;
const int intervalBuff = 300;

int maxY, maxX;
int lebar, tinggi, gameX, gameY; 
int rocketX, rocketY; 
int score = 0;
int fpsCounter = 0;
int currentFrameDelay = kecepatanNormal;
int lives = 1; 
int buffX = -1, buffY = -1;  
int jedaBuff = 200; 
int buffTimer = 0;  
int invincibilityTimer = 0; 

bool hasBuff = false; 
bool isInvincible = false; 

vector<pair<string, int>> scores;

vector<int> meteorX;
vector<int> meteorY;

int starX = -1, starY = -1;

void playMusic(const string &filename) {
    PlaySound(filename.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
}

void stopMusic() {
    PlaySound(NULL, NULL, 0);
}

void playlistMusic() {
    clear();
    string playlist[] = {
        "Megalovania",
        "TurboKiller",
        "RocketSpleef",
        "RetroPlatforming",
        "Back"
    };

    string files[] = {
        "Megalovania.wav",
        "TurboKiller.wav",
        "RocketSpleef.wav",
        "RetroPlatforming.wav"
    };

    int n_choices = sizeof(playlist) / sizeof(playlist[0]);
    int highlight = 0, choice = -1, c;

    WINDOW *music_win = newwin(15, 50, 7, 35);  
    keypad(music_win, TRUE);
    box(music_win, 0, 0);

    int win_width, win_height;
    getmaxyx(music_win, win_height, win_width);

    int start_y = (win_height - n_choices - 1) / 2;
    mvwprintw(music_win, 1, (win_width - 36) / 2, " _ \\ |               | _)       |  ");
    mvwprintw(music_win, 2, (win_width - 36) / 2, " __/ |   _` |  |  |  |  | (_-<   _|");
    mvwprintw(music_win, 3, (win_width - 36) / 2, "_|  _| \\__,_| \\_, | _| _| ___/ \\__|");
    mvwprintw(music_win, 4, (win_width - 36) / 2, "              ___/                 ");
    wrefresh(music_win);

    while (choice == -1) {
        for (int i = 0; i < n_choices; ++i) {
            int x_pos = (win_width - playlist[i].length()) / 2;
            if (i == highlight)
                wattron(music_win, A_REVERSE);
            mvwprintw(music_win, start_y + i + 2, x_pos, "%s", playlist[i].c_str());
            wattroff(music_win, A_REVERSE);
        }
        wrefresh(music_win);

        c = wgetch(music_win);

        switch (c) {
            case KEY_UP:
                highlight = (highlight == 0) ? n_choices - 1 : highlight - 1;
                break;
            case KEY_DOWN:
                highlight = (highlight == n_choices - 1) ? 0 : highlight + 1;
                break;
            case 10: 
                choice = highlight;
                break;
            default:
                break;
        }
    }

    if (choice < n_choices - 1) {
        clear();
        stopMusic(); 
        playMusic(files[choice]); 
        mvwprintw(music_win, win_height - 2, 2, "Playing: %s", playlist[choice].c_str());
        wrefresh(music_win);
        napms(1000);
        wclear(music_win);
    } else {
        clear();
        wclear(music_win);
    }
    wclear(music_win);
    clear();
    delwin(music_win);
}

void menuGame(WINDOW *menu_win, int highlight, const string choices[], int n_choices) {
    clear();
    int win_width, win_height;
    getmaxyx(menu_win, win_height, win_width);

    int title_pos_x = (win_width - 70) / 2; 
    mvwprintw(menu_win, 1, title_pos_x, " ________            ______      _____  ________              ______  ");
    mvwprintw(menu_win, 2, title_pos_x, " ___  __ \\______________  /________  /_ ___  __ \\___  ___________  /_ ");
    mvwprintw(menu_win, 3, title_pos_x, " __  /_/ /  __ \\  ___/_  //_/  _ \\  __/ __  /_/ /  / / /_  ___/_  __ \\");
    mvwprintw(menu_win, 4, title_pos_x, " _  _, _// /_/ / /__ _  ,<  /  __/ /_   _  _, _// /_/ /_(__  )_  / / /");
    mvwprintw(menu_win, 5, title_pos_x, " /_/ |_| \\____/\\___/ /_/|_| \\___/\\__/   /_/ |_| \\__,_/ /____/ /_/ /_/ ");
    
    mvwhline(menu_win, 6, 1, ACS_HLINE, win_width - 2);
    
    int start_y = 9;  
    for (int i = 0; i < n_choices; ++i) {
        int x_pos = (win_width - choices[i].length()) / 2; 
        if (i == highlight)
            wattron(menu_win, A_REVERSE); 
            clear();
        mvwprintw(menu_win, start_y + i, x_pos, "%s", choices[i].c_str());
        wattroff(menu_win, A_REVERSE);
    }
    box(menu_win, 0, 0);  
    wrefresh(menu_win);   
}

void listWarna() {
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);  
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); 
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_WHITE, COLOR_BLACK);   
    init_pair(5, COLOR_GREEN, COLOR_BLACK);  
    init_pair(6, COLOR_YELLOW, COLOR_BLACK);
    init_pair(7, COLOR_CYAN, COLOR_BLACK); 
}

void initializeGame() {
    clear();
    initscr();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(0);
    getmaxyx(stdscr, maxY, maxX);

    tinggi = maxY - 6;
    lebar = maxX - 20;
    gameY = 3;
    gameX = 10;

    rocketX = gameX + lebar / 2;
    rocketY = gameY + tinggi - 6;

    srand(time(0));

    lives = 1;
    buffX = -1;
    buffY = -1;
    hasBuff = false;
    isInvincible = false;
    invincibilityTimer = 0;
    score = 0;
    
    listWarna();

}

void borderGame() {
    attron(COLOR_PAIR(5));
    for (int i = 0; i < tinggi; i++) {
        mvprintw(gameY + i, gameX - 2, "|");    
        mvprintw(gameY + i, gameX + lebar + 1, "|"); 
    }
    for (int i = 0; i < lebar + 2; i++) {
        mvprintw(gameY - 1, gameX - 1 + i, "-");      
        mvprintw(gameY + tinggi, gameX - 1 + i, "-");
    }

    for (int i = 0; i < maxY; i += 4) {
        mvprintw(i, 2, "*");
        mvprintw(i, maxX - 3, "*");
    }
    for (int i = 0; i < maxX; i += 6) {
        mvprintw(1, i, "o");
        mvprintw(maxY - 2, i, "o");
    }
    attroff(COLOR_PAIR(5));
}

void tampilanRocket() {
    if (isInvincible) {
        attron(COLOR_PAIR(7)); 
    } else {
        attron(COLOR_PAIR(4));  
    }
    
    mvprintw(rocketY, rocketX, "   ^ ");
    mvprintw(rocketY + 1, rocketX, "  / \\");
    mvprintw(rocketY + 2, rocketX, " | O |");
    mvprintw(rocketY + 3, rocketX, " | | |");
    mvprintw(rocketY + 4, rocketX, "/_____\\");
    
    if (isInvincible) {
        attroff(COLOR_PAIR(7));
    } else {
        attroff(COLOR_PAIR(4));
    }

    attron(COLOR_PAIR(6));
    mvprintw(rocketY + 5, rocketX + 2, "/|\\");
    attroff(COLOR_PAIR(6));
}

void tampilanMeteor() {
    attron(COLOR_PAIR(3));
    for (size_t i = 0; i < meteorX.size(); i++) {
        if (meteorY[i] >= gameY && meteorY[i] < gameY + tinggi) {
            mvprintw(meteorY[i], meteorX[i],     "  O  ");
            mvprintw(meteorY[i] + 1, meteorX[i], "(   )");
            mvprintw(meteorY[i] + 2, meteorX[i], "  O  ");
        }
    }
    attroff(COLOR_PAIR(3));
}

void tampilanStar() {
    if (starY >= gameY && starY < gameY + tinggi) {
        attron(COLOR_PAIR(2));
        mvprintw(starY, starX,     " * ");
        mvprintw(starY + 1, starX, "***");
        mvprintw(starY + 2, starX, " * ");
        attroff(COLOR_PAIR(2));
    }
}
 
void buff() {
    if (buffY >= gameY && buffY < gameY + tinggi) {
        attron(COLOR_PAIR(2));
        mvprintw(buffY, buffX,     ">+<");
        mvprintw(buffY + 1, buffX, " ^ ");
        attroff(COLOR_PAIR(2));
    }
}

void updateObjects() {
    for (size_t i = 0; i < meteorX.size(); i++) {
        meteorY[i]++;
        if (meteorY[i] >= gameY + tinggi) {
            meteorY[i] = gameY - 3; 
            meteorX[i] = gameX + rand() % (lebar - 7);
        }
    }

    if (buffY >= 0) {
        buffY++;
        if (buffY >= gameY + tinggi) {
            buffY = -1;  
        }
    }

    buffTimer++;
    if (buffTimer >= jedaBuff) {
        buffX = gameX + rand() % (lebar - 7);
        buffY = gameY - 1; 
        buffTimer = 0; 
    }

    if (starY >= 0) {
        starY++;
        if (starY >= gameY + tinggi) {
            starY = -1;
        }
    }

    if (fpsCounter % jedaBintang == 0 && starY == -1) {
        starX = gameX + rand() % (lebar - 7);
        starY = gameY - 1;
    }

    if (isInvincible) {
        invincibilityTimer++;
        if (invincibilityTimer >= jedaBuff) {
            isInvincible = false;
            invincibilityTimer = 0;
        }
    }
}

void checkHitbox(bool &running) {
    int rocketLeft = rocketX;
    int rocketRight = rocketX + 6;
    int rocketTop = rocketY;
    int rocketBottom = rocketY + 4;

    for (size_t i = 0; i < meteorX.size(); i++) {
        int meteorLeft = meteorX[i];
        int meteorRight = meteorX[i] + 4;
        int meteorTop = meteorY[i];
        int meteorBottom = meteorY[i] + 2;

        if (meteorRight >= rocketLeft && meteorLeft <= rocketRight &&
            meteorBottom >= rocketTop && meteorTop <= rocketBottom) {
            if (lives > 0) {
                lives--;  
                meteorY[i] = gameY - 3; 
            } else {
                running = false; 
                return;
            }
        }
    }

    if (buffY >= 0) {
        int buffLeft = buffX;
        int buffRight = buffX + 2;
        int buffTop = buffY;
        int buffBottom = buffY + 1;

        if (buffRight >= rocketLeft && buffLeft <= rocketRight &&
            buffBottom >= rocketTop && buffTop <= rocketBottom) {
            lives += 1;
            isInvincible = true;
            invincibilityTimer = 0;
            buffY = -1;
            score += 50;
        }
    }

    int starLeft = starX;
    int starRight = starX + 2;
    int starTop = starY;
    int starBottom = starY + 2;

    if (starRight >= rocketLeft && starLeft <= rocketRight &&
        starBottom >= rocketTop && starTop <= rocketBottom) {
        score += 44;
        starY = -1;
    }
}

void drawHUD() {
    mvprintw(1, gameX + lebar / 2 - 5, "SCORE: %d", score);
    mvprintw(1, gameX, "LIVES: %d", lives);
    if (isInvincible) {
        attron(COLOR_PAIR(7));
        mvprintw(1, gameX + lebar - 20, "INVINCIBLE!");
        attroff(COLOR_PAIR(7));
    }
}

void handleInput(bool &running) {
    int ch = getch();
    switch (ch) {
        case KEY_LEFT:
            if (rocketX > gameX) rocketX = rocketX - 2;
            break;
        case KEY_RIGHT:
            if (rocketX + 6 < gameX + lebar) rocketX = rocketX + 2;
            break;
        case 'q':
            running = false;
            break;
    }
}

void updateSpeed() {
    if (score % 1000 == 0) {
        currentFrameDelay = kecepatanNormal;
    } else if (score % kelipatanSkor == 0) {
        currentFrameDelay -= 5;
        if (currentFrameDelay < 20) currentFrameDelay = 20;
    }
}

void saveScore(const string &name, int points) {
    ofstream outFile("scores.txt", ios::app); 
    if (outFile.is_open()) {
        outFile << name << " " << points << "\n";
        outFile.close();
    }
}

void loadScores() {
    scores.clear();
    ifstream inFile("scores.txt");
    string line;

    while (getline(inFile, line)) {
        istringstream iss(line);
        string name;
        int points;

        if (iss >> name >> points) {
            scores.push_back(make_pair(name, points));
        }
    }
    inFile.close();

    sort(scores.begin(), scores.end(), [](const pair<string, int> &a, const pair<string, int> &b) {
        return b.second < a.second;
    });
}

void showLeaderboard() {
    loadScores(); 
    clear();
    WINDOW *leaderboardWin = newwin(20, 80, 5, 19);
    box(leaderboardWin, 0, 0);

    mvwprintw(leaderboardWin, 1, 8, "  |                   |           |                         |       ");
    mvwprintw(leaderboardWin, 2, 8, "  |   -_)   _` |   _` |   -_)   _| _ \\   _ \\   _` |   _| _` |     ");
    mvwprintw(leaderboardWin, 3, 8, " _| \\___| \\__,_| \\__,_| \\___| _| _.__/ \\___/ \\__,_| _| \\__,_|");
    mvwprintw(leaderboardWin, 4, 8, " ============================================================");
    mvwprintw(leaderboardWin, 5, 8, "                      Highscore Leaderboard                       ");

    for (size_t i = 0; i < scores.size() && i < 10; ++i) {
        mvwprintw(leaderboardWin, 7 + i, 2, "%d. \t%s \t-   %d", i + 1, scores[i].first.c_str(), scores[i].second);
    }

    mvwprintw(leaderboardWin, 18, 2, "Press any key to return...");
    wrefresh(leaderboardWin);
    wgetch(leaderboardWin);
    delwin(leaderboardWin);
    clear();
}

void resetGame() {
    score = 0;
    currentFrameDelay = kecepatanNormal;
    rocketX = gameX + lebar / 2; 
    rocketY = gameY + tinggi - 6;
    meteorX.clear();   
    meteorY.clear();
    starX = -1;  
    starY = -1;
    fpsCounter = 0;
    timeout(-1);
    clear();
}

void gameOverScreen() {
    clear();
    int centerY = maxY / 2; 
    int centerX = maxX / 2;

    attron(COLOR_PAIR(5)); 
    mvprintw(centerY - 4, centerX - 32, "_________                          _______                       ");
    mvprintw(centerY - 3, centerX - 32, "__  ____/_____ _______ ________    __  __ \\__   ______________  ");
    mvprintw(centerY - 2, centerX - 32, "_  / __ _  __ `/_  __ `__ \\  _ \\   _  / / /_ | / /  _ \\_  ___/");
    mvprintw(centerY - 1, centerX - 32, "/ /_/ / / /_/ /_  / / / / /  __/   / /_/ /__ |/ //  __/  /       "); 
    mvprintw(centerY,     centerX - 32, "\\____/  \\__,_/ /_/ /_/ /_/\\___/    \\____/ _____/ \\___//_/   "); 
    mvprintw(centerY + 2, centerX - 10, "Score Anda: %d", score);
    attroff(COLOR_PAIR(5)); 
    
    timeout(-1);
    cbreak();
    curs_set(1);
    char playerName[50];
    
    mvprintw(centerY + 3, centerX - 10, "Enter your name: ");
    move(centerY + 3, centerX + 7);
    echo(); 
    mvgetnstr(centerY + 3, centerX + 7, playerName, sizeof(playerName) - 1);  
    noecho();
    curs_set(0);

    saveScore(playerName, score);
    refresh();
    resetGame();
}

void gameLoop() {
    bool running = true;
    while (running) {
        clear();
        borderGame();
        tampilanRocket();
        tampilanMeteor();
        buff();
        tampilanStar();
        refresh();
        drawHUD();
        handleInput(running);
        updateObjects();
        checkHitbox(running);
        updateSpeed();

        if (fpsCounter % fps == 0) {
            score += 4;
        }

        if (meteorX.size() < limitMeteor && fpsCounter % 20 == 0) {
            meteorX.push_back(gameX + rand() % (lebar - 3));
            meteorY.push_back(gameY - 3);
        }

        napms(currentFrameDelay);
        fpsCounter++;

        if (lives == 0) {
            running = false; 
        }
    }
    gameOverScreen(); 
    stopMusic(); 
    clear();
}

int main() {
    initscr();
    clear();
    noecho();
    cbreak();
    curs_set(0); 

    int highlight = 0;
    int choice = -1;
    int c;

    string menuItems[] = {"Start Game", "Game Music", "Highscores", "Exit"};
    int n_choices = sizeof(menuItems) / sizeof(menuItems[0]);

    WINDOW *menu_win = newwin(15, 80, 7, 19);
    keypad(menu_win, TRUE);

    while (true) {
        refresh();
        clear();
        menuGame(menu_win, highlight, menuItems, n_choices);
        c = wgetch(menu_win);

        switch (c) {
            case KEY_UP:
                highlight = (highlight == 0) ? n_choices - 1 : highlight - 1;
                break;
            case KEY_DOWN:
                highlight = (highlight == n_choices - 1) ? 0 : highlight + 1;
                break;
            case 10:
                choice = highlight;
                break;
            default:
                break;
        }

        if (choice != -1) {
            if (choice == 0) {
                refresh();
                clear();
                initializeGame();
                gameLoop();
                clear();
            } else if (choice == 1) {
                clear();
                refresh();
                playlistMusic();
                clear();
            } else if (choice == 2) {
                clear();
                refresh();
                showLeaderboard(); 
                clear();
            } else if (choice == 3) {
                mvwprintw(menu_win, 12, 2, "Exiting Program...");
                wrefresh(menu_win);
                napms(1000);
                clear();
                break;
            }
            clear();
            wrefresh(menu_win);
            napms(1000);
            choice = -1;
            wclear(menu_win);
            box(menu_win, 0, 0);
        }
    }

    refresh();
    clear();
    delwin(menu_win);
    endwin();
    return 0;
}