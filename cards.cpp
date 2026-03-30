#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#endif

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <random>
#include <algorithm>
#include <memory>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <cctype>
#include <functional>

class Random {
    std::mt19937 rng;
public:
    Random() : rng(std::random_device{}()) {}
    int get(int min, int max) { return std::uniform_int_distribution<int>(min, max)(rng); }
    double getDouble(double min, double max) { return std::uniform_real_distribution<double>(min, max)(rng); }
    bool chance(double percent) { return getDouble(0.0, 100.0) < percent; }
};

Random g_random;

class Console {
public:
    static void clear() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }
    
    static void setColor(int color) {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, color);
#else
        std::cout << "\033[" << color << "m";
#endif
    }
    
    static void resetColor() {
#ifdef _WIN32
        setColor(7);
#else
        std::cout << "\033[0m";
#endif
    }
    
    static void gotoxy(int x, int y) {
#ifdef _WIN32
        COORD coord;
        coord.X = x;
        coord.Y = y;
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#else
        std::cout << "\033[" << y << ";" << x << "H";
#endif
    }
    
    static void hideCursor() {
#ifdef _WIN32
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
#else
        std::cout << "\033[?25l";
#endif
    }
    
    static void showCursor() {
#ifdef _WIN32
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
#else
        std::cout << "\033[?25h";
#endif
    }
    
    static char getKey() {
#ifdef _WIN32
        return _getch();
#else
        struct termios oldt, newt;
        char ch;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
#endif
    }
    
    static void sleep(int ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    
    static void drawBox(int x, int y, int width, int height, int color = 7) {
        setColor(color);
        gotoxy(x, y); std::cout << "+" << std::string(width - 2, '-') << "+";
        for (int i = 1; i < height - 1; i++) {
            gotoxy(x, y + i); std::cout << "|" << std::string(width - 2, ' ') << "|";
        }
        gotoxy(x, y + height - 1); std::cout << "+" << std::string(width - 2, '-') << "+";
        resetColor();
    }
    
    static void drawProgress(int x, int y, int width, int current, int max, int color = 10) {
        int filled = (width - 2) * current / max;
        setColor(color);
        gotoxy(x, y); std::cout << "[";
        for (int i = 0; i < width - 2; i++) {
            if (i < filled) std::cout << "=";
            else if (i == filled) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "]";
        resetColor();
    }
};

class Card {
public:
    std::string name;
    std::string type;
    int hp;
    int maxHp;
    int attack;
    int defense;
    int level;
    int exp;
    bool isEvolved;
    
    Card() : name(""), type("Normal"), hp(0), maxHp(0), attack(0), defense(0), level(1), exp(0), isEvolved(false) {}
    
    Card(std::string n, std::string t, int h, int a, int d) 
        : name(n), type(t), hp(h), maxHp(h), attack(a), defense(d), level(1), exp(0), isEvolved(false) {}
    
    virtual void evolve() {
        if (!isEvolved) {
            maxHp = maxHp * 1.5;
            hp = maxHp;
            attack = attack * 1.3;
            defense = defense * 1.2;
            level++;
            isEvolved = true;
        }
    }
    
    virtual void takeDamage(int damage) {
        int actualDamage = std::max(1, damage - defense / 5);
        hp = std::max(0, hp - actualDamage);
    }
    
    virtual int calculateDamage() {
        return attack + g_random.get(-5, 5);
    }
    
    virtual void gainExp(int amount) {
        exp += amount;
        if (exp >= 100 && level < 5) {
            exp = 0;
            level++;
            maxHp += 10;
            hp = maxHp;
            attack += 5;
            defense += 3;
        }
    }
    
    virtual bool isAlive() const { return hp > 0; }
    virtual std::string getStatus() const {
        std::ostringstream oss;
        oss << name << " Lv." << level << " [" << hp << "/" << maxHp << "]";
        return oss.str();
    }
    
    virtual std::unique_ptr<Card> clone() const {
        return std::make_unique<Card>(*this);
    }
    
    virtual void save(std::ofstream& file) const {
        file << "Card " << name << " " << type << " " << maxHp << " " << attack << " " << defense << " " << level << " " << exp << " " << isEvolved << "\n";
    }
    
    virtual void load(std::ifstream& file) {
        file >> name >> type >> maxHp >> attack >> defense >> level >> exp >> isEvolved;
        hp = maxHp;
    }
    
    virtual ~Card() = default;
};

class FireCard : public Card {
public:
    FireCard(std::string n, int h, int a, int d) : Card(n, "Fire", h, a, d) {}
    
    int calculateDamage() override {
        return attack + g_random.get(0, 10) + (type == "Fire" ? 3 : 0);
    }
    
    void takeDamage(int damage) override {
        int actualDamage = std::max(1, damage - defense / 4);
        hp = std::max(0, hp - actualDamage);
    }
    
    std::unique_ptr<Card> clone() const override {
        return std::make_unique<FireCard>(*this);
    }
    
    ~FireCard() override = default;
};

class WaterCard : public Card {
public:
    WaterCard(std::string n, int h, int a, int d) : Card(n, "Water", h, a, d) {}
    
    int calculateDamage() override {
        int healChance = g_random.get(1, 100);
        if (healChance <= 20) {
            hp = std::min(maxHp, hp + 5);
        }
        return attack + g_random.get(-3, 8);
    }
    
    std::unique_ptr<Card> clone() const override {
        return std::make_unique<WaterCard>(*this);
    }
    
    ~WaterCard() override = default;
};

class GrassCard : public Card {
public:
    GrassCard(std::string n, int h, int a, int d) : Card(n, "Grass", h, a, d) {}
    
    void evolve() override {
        if (!isEvolved) {
            maxHp = maxHp * 1.8;
            hp = maxHp;
            attack = attack * 1.1;
            defense = defense * 1.4;
            level++;
            isEvolved = true;
        }
    }
    
    std::unique_ptr<Card> clone() const override {
        return std::make_unique<GrassCard>(*this);
    }
    
    ~GrassCard() override = default;
};

class ElectricCard : public Card {
public:
    ElectricCard(std::string n, int h, int a, int d) : Card(n, "Electric", h, a, d) {}
    
    int calculateDamage() override {
        int crit = g_random.get(1, 100);
        int dmg = attack + g_random.get(-2, 12);
        if (crit <= 15) {
            dmg *= 2;
        }
        return dmg;
    }
    
    std::unique_ptr<Card> clone() const override {
        return std::make_unique<ElectricCard>(*this);
    }
    
    ~ElectricCard() override = default;
};

class RockCard : public Card {
public:
    RockCard(std::string n, int h, int a, int d) : Card(n, "Rock", h, a, d) {}
    
    void takeDamage(int damage) override {
        int actualDamage = std::max(1, damage - defense / 2);
        hp = std::max(0, hp - actualDamage);
    }
    
    std::unique_ptr<Card> clone() const override {
        return std::make_unique<RockCard>(*this);
    }
    
    ~RockCard() override = default;
};

class Player {
public:
    std::string name;
    std::vector<std::unique_ptr<Card>> deck;
    std::vector<std::unique_ptr<Card>> collection;
    int wins;
    int losses;
    int coins;
    
    Player() : name("Trainer"), wins(0), losses(0), coins(100) {}
    
    void addCardToCollection(std::unique_ptr<Card> card) {
        collection.push_back(std::move(card));
    }
    
    void buildDeck() {
        deck.clear();
        if (collection.empty()) return;
        std::vector<int> indices(collection.size());
        for (size_t i = 0; i < collection.size(); i++) indices[i] = i;
        std::shuffle(indices.begin(), indices.end(), std::mt19937(std::random_device{}()));
        for (int i = 0; i < std::min(6, (int)collection.size()); i++) {
            deck.push_back(collection[indices[i]]->clone());
        }
    }
    
    void healDeck() {
        for (auto& card : deck) {
            card->hp = card->maxHp;
        }
    }
    
    bool hasCards() const {
        return !deck.empty();
    }
    
    void save(std::ofstream& file) const {
        file << "Player " << name << " " << wins << " " << losses << " " << coins << " " << collection.size() << "\n";
        for (const auto& card : collection) {
            card->save(file);
        }
    }
    
    void load(std::ifstream& file) {
        std::string type;
        int count;
        file >> type >> name >> wins >> losses >> coins >> count;
        collection.clear();
        for (int i = 0; i < count; i++) {
            std::string cardType, cardName, cardElement;
            int hp, atk, def, lvl, exp;
            bool evolved;
            file >> cardType >> cardName >> cardElement >> hp >> atk >> def >> lvl >> exp >> evolved;
            std::unique_ptr<Card> newCard;
            if (cardElement == "Fire") newCard = std::make_unique<FireCard>(cardName, hp, atk, def);
            else if (cardElement == "Water") newCard = std::make_unique<WaterCard>(cardName, hp, atk, def);
            else if (cardElement == "Grass") newCard = std::make_unique<GrassCard>(cardName, hp, atk, def);
            else if (cardElement == "Electric") newCard = std::make_unique<ElectricCard>(cardName, hp, atk, def);
            else if (cardElement == "Rock") newCard = std::make_unique<RockCard>(cardName, hp, atk, def);
            else newCard = std::make_unique<Card>(cardName, cardElement, hp, atk, def);
            newCard->level = lvl;
            newCard->exp = exp;
            newCard->isEvolved = evolved;
            newCard->hp = newCard->maxHp;
            collection.push_back(std::move(newCard));
        }
        buildDeck();
    }
};

class Battle {
    Player& player;
    Player& opponent;
    int playerCardIndex;
    int opponentCardIndex;
    bool playerTurn;
    std::string battleLog;
    
public:
    Battle(Player& p, Player& o) : player(p), opponent(o), playerCardIndex(0), opponentCardIndex(0), playerTurn(true) {
        player.buildDeck();
        opponent.buildDeck();
        if (!player.deck.empty()) playerCardIndex = 0;
        if (!opponent.deck.empty()) opponentCardIndex = 0;
        battleLog = "Battle started!";
    }
    
    void render() {
        Console::clear();
        Console::setColor(11);
        Console::gotoxy(35, 1); std::cout << "=== POKEMON BATTLE ===";
        Console::resetColor();
        
        Console::drawBox(2, 3, 40, 8, 14);
        Console::gotoxy(4, 4); std::cout << opponent.name;
        if (!opponent.deck.empty() && opponentCardIndex < (int)opponent.deck.size()) {
            auto& card = opponent.deck[opponentCardIndex];
            Console::gotoxy(4, 5); std::cout << card->getStatus();
            Console::drawProgress(4, 6, 30, card->hp, card->maxHp, 12);
            Console::gotoxy(4, 7); std::cout << "ATK:" << card->attack << " DEF:" << card->defense;
        } else {
            Console::gotoxy(4, 5); std::cout << "No active card!";
        }
        
        Console::drawBox(2, 15, 40, 8, 10);
        Console::gotoxy(4, 16); std::cout << player.name;
        if (!player.deck.empty() && playerCardIndex < (int)player.deck.size()) {
            auto& card = player.deck[playerCardIndex];
            Console::gotoxy(4, 17); std::cout << card->getStatus();
            Console::drawProgress(4, 18, 30, card->hp, card->maxHp, 10);
            Console::gotoxy(4, 19); std::cout << "ATK:" << card->attack << " DEF:" << card->defense;
        } else {
            Console::gotoxy(4, 17); std::cout << "No active card!";
        }
        
        Console::drawBox(45, 3, 35, 20, 8);
        Console::gotoxy(47, 4); std::cout << "BATTLE LOG";
        std::string log = battleLog;
        for (int i = 0; i < 10; i++) {
            Console::gotoxy(47, 6 + i);
            std::cout << std::setw(30) << std::left << (log.length() > 30 ? log.substr(0, 30) : log);
            log.clear();
        }
        
        Console::drawBox(2, 24, 78, 5, 7);
        Console::gotoxy(4, 25); std::cout << "[A] Attack  [S] Switch  [E] Evolve  [R] Run  [Q] Quit";
        if (!playerTurn) {
            Console::gotoxy(4, 27); std::cout << "Opponent's turn...";
        }
    }
    
    void log(const std::string& msg) {
        battleLog = msg;
    }
    
    bool playerAction(char action) {
        if (!playerTurn) return true;
        if (player.deck.empty()) return false;
        
        auto& playerCard = player.deck[playerCardIndex];
        auto& opponentCard = opponent.deck[opponentCardIndex];
        
        switch (action) {
            case 'a':
            case 'A': {
                int damage = playerCard->calculateDamage();
                opponentCard->takeDamage(damage);
                log(playerCard->name + " deals " + std::to_string(damage) + " damage!");
                if (!opponentCard->isAlive()) {
                    log(opponentCard->name + " fainted!");
                    opponent.deck.erase(opponent.deck.begin() + opponentCardIndex);
                    if (opponent.deck.empty()) return false;
                    opponentCardIndex = 0;
                }
                playerTurn = false;
                break;
            }
            case 's':
            case 'S': {
                if (player.deck.size() <= 1) {
                    log("No cards to switch with!");
                    return true;
                }
                int newIndex = (playerCardIndex + 1) % player.deck.size();
                playerCardIndex = newIndex;
                log("Switched to " + player.deck[playerCardIndex]->name);
                playerTurn = false;
                break;
            }
            case 'e':
            case 'E': {
                if (!playerCard->isEvolved && playerCard->level >= 3) {
                    playerCard->evolve();
                    log(playerCard->name + " evolved!");
                } else {
                    log("Cannot evolve yet! Need level 3.");
                }
                playerTurn = false;
                break;
            }
            case 'r':
            case 'R': {
                if (g_random.chance(50)) {
                    log("You escaped successfully!");
                    return false;
                } else {
                    log("Failed to escape!");
                    playerTurn = false;
                }
                break;
            }
            default:
                return true;
        }
        return true;
    }
    
    void opponentTurn() {
        if (playerTurn) return;
        if (opponent.deck.empty() || player.deck.empty()) return;
        
        auto& playerCard = player.deck[playerCardIndex];
        auto& opponentCard = opponent.deck[opponentCardIndex];
        
        Console::sleep(800);
        int damage = opponentCard->calculateDamage();
        playerCard->takeDamage(damage);
        log(opponentCard->name + " deals " + std::to_string(damage) + " damage!");
        
        if (!playerCard->isAlive()) {
            log(playerCard->name + " fainted!");
            player.deck.erase(player.deck.begin() + playerCardIndex);
            if (player.deck.empty()) return;
            playerCardIndex = 0;
        }
        playerTurn = true;
    }
    
    bool update() {
        if (player.deck.empty()) return false;
        if (opponent.deck.empty()) return false;
        return true;
    }
    
    std::string getWinner() {
        if (player.deck.empty()) return opponent.name;
        if (opponent.deck.empty()) return player.name;
        return "";
    }
    
    bool isPlayerTurn() const {
        return playerTurn;
    }
    
    void executeOpponentTurn() {
        opponentTurn();
    }
};

class Shop {
public:
    static void show(Player& player) {
        bool shopping = true;
        std::vector<std::unique_ptr<Card>> shopCards;
        shopCards.push_back(std::make_unique<FireCard>("Charmander", 50, 25, 15));
        shopCards.push_back(std::make_unique<WaterCard>("Squirtle", 55, 22, 18));
        shopCards.push_back(std::make_unique<GrassCard>("Bulbasaur", 60, 20, 20));
        shopCards.push_back(std::make_unique<ElectricCard>("Pikachu", 45, 28, 12));
        shopCards.push_back(std::make_unique<RockCard>("Geodude", 65, 24, 22));
        
        while (shopping) {
            Console::clear();
            Console::setColor(14);
            Console::gotoxy(35, 1); std::cout << "=== POKEMON SHOP ===";
            Console::resetColor();
            Console::gotoxy(2, 3); std::cout << "Coins: " << player.coins;
            
            for (size_t i = 0; i < shopCards.size(); i++) {
                Console::drawBox(2, 5 + i * 6, 70, 5, 7);
                Console::gotoxy(4, 6 + i * 6); 
                std::cout << (char)('1' + i) << ". " << shopCards[i]->name << " (" << shopCards[i]->type << ")";
                Console::gotoxy(4, 7 + i * 6);
                std::cout << "   HP:" << shopCards[i]->maxHp << " ATK:" << shopCards[i]->attack << " DEF:" << shopCards[i]->defense;
                Console::gotoxy(4, 8 + i * 6);
                std::cout << "   Price: 50 coins";
            }
            
            Console::drawBox(2, 5 + shopCards.size() * 6, 70, 3, 7);
            Console::gotoxy(4, 6 + shopCards.size() * 6);
            std::cout << "[Q] Exit shop";
            
            Console::gotoxy(2, 10 + shopCards.size() * 6);
            std::cout << "Select option: ";
            char choice = Console::getKey();
            
            if (choice == 'q' || choice == 'Q') {
                shopping = false;
            } else if (choice >= '1' && choice <= '5') {
                int idx = choice - '1';
                if (player.coins >= 50) {
                    player.coins -= 50;
                    player.addCardToCollection(shopCards[idx]->clone());
                    Console::gotoxy(2, 12 + shopCards.size() * 6);
                    std::cout << "Purchased " << shopCards[idx]->name << "!                    ";
                    Console::sleep(1000);
                } else {
                    Console::gotoxy(2, 12 + shopCards.size() * 6);
                    std::cout << "Not enough coins!                      ";
                    Console::sleep(1000);
                }
            }
        }
    }
};

class Game {
    Player player;
    std::vector<Player> trainers;
    int currentTrainer;
    
    void loadTrainers() {
        trainers.clear();
        Player rival;
        rival.name = "Blue";
        rival.addCardToCollection(std::make_unique<FireCard>("Growlithe", 65, 28, 20));
        rival.addCardToCollection(std::make_unique<WaterCard>("Starmie", 70, 30, 25));
        rival.addCardToCollection(std::make_unique<GrassCard>("Tangela", 75, 22, 30));
        rival.buildDeck();
        trainers.push_back(std::move(rival));
        
        Player gym;
        gym.name = "Brock";
        gym.addCardToCollection(std::make_unique<RockCard>("Onix", 90, 35, 40));
        gym.addCardToCollection(std::make_unique<RockCard>("Geodude", 70, 28, 35));
        gym.addCardToCollection(std::make_unique<RockCard>("Rhyhorn", 85, 32, 38));
        gym.buildDeck();
        trainers.push_back(std::move(gym));
        
        Player elite;
        elite.name = "Lance";
        elite.addCardToCollection(std::make_unique<FireCard>("Dragonite", 120, 45, 40));
        elite.addCardToCollection(std::make_unique<ElectricCard>("Gyarados", 110, 48, 35));
        elite.addCardToCollection(std::make_unique<WaterCard>("Aerodactyl", 100, 42, 38));
        elite.buildDeck();
        trainers.push_back(std::move(elite));
        
        Player champ;
        champ.name = "Champion";
        champ.addCardToCollection(std::make_unique<FireCard>("Charizard", 150, 55, 45));
        champ.addCardToCollection(std::make_unique<WaterCard>("Blastoise", 160, 50, 50));
        champ.addCardToCollection(std::make_unique<GrassCard>("Venusaur", 170, 48, 55));
        champ.addCardToCollection(std::make_unique<ElectricCard>("Zapdos", 140, 60, 42));
        champ.buildDeck();
        trainers.push_back(std::move(champ));
    }
    
public:
    Game() : currentTrainer(0) {
        loadTrainers();
        player.name = "Ash";
        player.addCardToCollection(std::make_unique<FireCard>("Charmander", 50, 25, 15));
        player.addCardToCollection(std::make_unique<WaterCard>("Squirtle", 55, 22, 18));
        player.addCardToCollection(std::make_unique<GrassCard>("Bulbasaur", 60, 20, 20));
        player.buildDeck();
    }
    
    void saveGame() {
        std::ofstream file("pokemon_save.dat");
        if (file.is_open()) {
            player.save(file);
            file << trainers.size() << "\n";
            for (const auto& t : trainers) {
                t.save(file);
            }
            file << currentTrainer << "\n";
            file.close();
        }
    }
    
    void loadGame() {
        std::ifstream file("pokemon_save.dat");
        if (file.is_open()) {
            player.load(file);
            int count;
            file >> count;
            trainers.clear();
            for (int i = 0; i < count; i++) {
                Player t;
                t.load(file);
                trainers.push_back(std::move(t));
            }
            file >> currentTrainer;
            file.close();
        }
    }
    
    void mainMenu() {
        while (true) {
            Console::clear();
            Console::setColor(13);
            Console::gotoxy(30, 5); std::cout << "╔════════════════════════════════════╗";
            Console::gotoxy(30, 6); std::cout << "║      POKEMON CARD BATTLE v2.0     ║";
            Console::gotoxy(30, 7); std::cout << "╚════════════════════════════════════╝";
            Console::resetColor();
            Console::gotoxy(35, 10); std::cout << "1. New Game";
            Console::gotoxy(35, 11); std::cout << "2. Load Game";
            Console::gotoxy(35, 12); std::cout << "3. Collection";
            Console::gotoxy(35, 13); std::cout << "4. Shop";
            Console::gotoxy(35, 14); std::cout << "5. Battle";
            Console::gotoxy(35, 15); std::cout << "6. Quit";
            Console::gotoxy(35, 17); std::cout << "Choose: ";
            
            char choice = Console::getKey();
            if (choice == '1') {
                loadTrainers();
                player = Player();
                player.name = "Ash";
                player.addCardToCollection(std::make_unique<FireCard>("Charmander", 50, 25, 15));
                player.addCardToCollection(std::make_unique<WaterCard>("Squirtle", 55, 22, 18));
                player.addCardToCollection(std::make_unique<GrassCard>("Bulbasaur", 60, 20, 20));
                player.buildDeck();
                currentTrainer = 0;
                break;
            } else if (choice == '2') {
                loadGame();
                break;
            } else if (choice == '3') {
                showCollection();
            } else if (choice == '4') {
                Shop::show(player);
            } else if (choice == '5') {
                if (!player.hasCards()) {
                    Console::gotoxy(35, 19);
                    std::cout << "No cards! Visit shop first!";
                    Console::sleep(1500);
                } else {
                    startBattle();
                }
            } else if (choice == '6') {
                exit(0);
            }
        }
    }
    
    void showCollection() {
        while (true) {
            Console::clear();
            Console::setColor(11);
            Console::gotoxy(30, 1); std::cout << "=== CARD COLLECTION ===";
            Console::resetColor();
            Console::gotoxy(2, 3); std::cout << "Trainer: " << player.name << " | Wins: " << player.wins << " | Losses: " << player.losses << " | Coins: " << player.coins;
            Console::gotoxy(2, 4); std::cout << "Cards owned: " << player.collection.size();
            
            int y = 6;
            for (size_t i = 0; i < player.collection.size() && i < 20; i++) {
                auto& card = player.collection[i];
                Console::gotoxy(2, y++);
                std::cout << i+1 << ". " << card->name << " (" << card->type << ") Lv." << card->level 
                          << " HP:" << card->maxHp << " ATK:" << card->attack << " DEF:" << card->defense;
                if (card->isEvolved) std::cout << " [EVOLVED]";
            }
            
            Console::drawBox(2, 27, 76, 3, 7);
            Console::gotoxy(4, 28); std::cout << "[B] Build Deck  [D] View Deck  [Q] Back";
            Console::gotoxy(4, 29); std::cout << "Choice: ";
            char ch = Console::getKey();
            if (ch == 'q' || ch == 'Q') break;
            else if (ch == 'b' || ch == 'B') {
                player.buildDeck();
                Console::gotoxy(4, 30); std::cout << "Deck built with 6 random cards!      ";
                Console::sleep(1000);
            } else if (ch == 'd' || ch == 'D') {
                Console::clear();
                Console::gotoxy(30, 1); std::cout << "=== CURRENT DECK ===";
                for (size_t i = 0; i < player.deck.size(); i++) {
                    Console::gotoxy(2, 3 + i);
                    std::cout << i+1 << ". " << player.deck[i]->getStatus();
                }
                Console::gotoxy(2, 15); std::cout << "Press any key to continue...";
                Console::getKey();
            }
        }
    }
    
    void startBattle() {
        if (currentTrainer >= (int)trainers.size()) {
            Console::clear();
            Console::gotoxy(30, 10); std::cout << "You are the Pokemon Champion!";
            Console::gotoxy(30, 12); std::cout << "Credits roll...";
            Console::sleep(3000);
            currentTrainer = 0;
            return;
        }
        
        Player& opponent = trainers[currentTrainer];
        player.healDeck();
        opponent.healDeck();
        
        Battle battle(player, opponent);
        bool inBattle = true;
        
        while (inBattle) {
            battle.render();
            if (!battle.update()) {
                inBattle = false;
                break;
            }
            
            if (battle.isPlayerTurn()) {
                char key = Console::getKey();
                if (key == 'q' || key == 'Q') {
                    inBattle = false;
                    continue;
                }
                inBattle = battle.playerAction(key);
                if (!inBattle) {
                    if (battle.getWinner().empty()) {
                        player.wins++;
                        player.coins += 50;
                        currentTrainer++;
                        Console::clear();
                        Console::gotoxy(30, 10); std::cout << "You defeated " << opponent.name << "!";
                        Console::gotoxy(30, 11); std::cout << "Reward: 50 coins";
                        Console::sleep(2000);
                    } else {
                        player.losses++;
                        Console::clear();
                        Console::gotoxy(30, 10); std::cout << "You lost to " << opponent.name << "...";
                        Console::sleep(2000);
                    }
                    saveGame();
                }
            } else {
                battle.executeOpponentTurn();
                inBattle = battle.update();
                if (!inBattle) {
                    player.wins++;
                    player.coins += 50;
                    currentTrainer++;
                    Console::clear();
                    Console::gotoxy(30, 10); std::cout << "Victory! Defeated " << opponent.name;
                    Console::sleep(2000);
                    saveGame();
                }
            }
        }
    }
    
    void run() {
        Console::hideCursor();
        mainMenu();
        while (true) {
            if (currentTrainer >= (int)trainers.size()) {
                Console::clear();
                Console::gotoxy(30, 10); std::cout << "Congratulations! You are the Champion!";
                Console::gotoxy(30, 12); std::cout << "Press any key to continue...";
                Console::getKey();
                currentTrainer = 0;
                mainMenu();
            } else {
                startBattle();
                mainMenu();
            }
        }
    }
};

int main() {
    try {
        Game game;
        game.run();
    } catch (const std::exception& e) {
        Console::resetColor();
        Console::showCursor();
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    Console::showCursor();
    return 0;
}