# cards 

A terminal-based card battle game inspired by Pokémon, where players collect cards, build decks, and battle against AI trainers. The game features multiple card types with unique abilities, a shop system, and persistent save functionality.

## Features

- **5 Card Types** with unique abilities:
  - **Fire**: Bonus damage, reduced defense effectiveness
  - **Water**: Chance to heal when attacking
  - **Grass**: Enhanced HP growth on evolution
  - **Electric**: Critical hit chance
  - **Rock**: High damage reduction

- **Evolution System**: Cards evolve at level 3, gaining enhanced stats
- **Progressive Difficulty**: Battle through 4 trainers (Rival, Brock, Lance, Champion)
- **Shop System**: Purchase new cards with coins earned from victories
- **Collection Management**: View your card collection and build custom decks
- **Save/Load**: Persistent game progress saved to file
- **Cross-Platform**: Works on Windows (conio.h) and Unix-like systems (termios)

## Installation

### Quick Install (Linux/macOS)
```bash
curl -O https://github.com/createdbyglitch/cards/raw/main/cards.cpp
g++ -std=c++17 -o pokemon cards.cpp
./pokemon
```

### Windows Installation

1. Download cards.cpp from the repository
2. Compile with MinGW or Visual Studio:
   ```bash
   g++ -std=c++17 -o pokemon.exe cards.cpp
   ```
3. Run pokemon.exe

## Requirements

- C++17 compatible compiler
- Terminal/console with ANSI color support (for Unix systems)
- Windows: Standard console with conio.h support

### How to Play

## Main Menu

1. New Game: Start fresh with starter cards (Charmander, Squirtle, Bulbasaur)
2. Load Game: Continue from saved progress
3. Collection: View cards, build deck (6 random cards from collection)
4. Shop: Purchase new cards (50 coins each)
5. Battle: Challenge the next trainer
6. Quit: Exit game

## Battle Controls

- A: Attack the opponent's active card
- S: Switch to another card in your deck
- E: Evolve current card (requires level 3)
- R: Attempt to escape battle (50% chance)
- Q: Quit battle (returns to menu)

## Card Evolution

- Cards gain experience points (EXP) from battles
- 100 EXP = 1 level up (max level 5)
- Evolution occurs at level 3, granting:
  - 50% HP increase
  - 30% Attack increase
  - 20% Defense increase

## Battle System

- Turn-based combat with alternating player and opponent turns
- Damage calculation: attack ± random(0-5) - (defense/5)
- Cards faint when HP reaches 0
- Battle ends when one player's deck is empty
- Winner receives 50 coins and progresses to next trainer

## Game Progression

Trainer Cards Difficulty
Blue Growlithe, Starmie, Tangela Beginner
Brock Onix, Geodude, Rhyhorn Intermediate
Lance Dragonite, Gyarados, Aerodactyl Advanced
Champion Charizard, Blastoise, Venusaur, Zapdos Expert

Save File Format

Game saves are stored in pokemon_save.dat in the current directory. The file contains:

- Player information (name, stats, coins)
- Full card collection with levels and evolution status
- Trainer progress data
- Current battle index

#### Building from Source

### Linux/macOS

```bash
g++ -std=c++17 -o pokemon cards.cpp -pthread
```

### Windows (MinGW)

```bash
g++ -std=c++17 -o pokemon.exe cards.cpp -static-libgcc -static-libstdc++
```

### Windows (Visual Studio)

```bash
cl /EHsc /std:c++17 cards.cpp
```

### License

This project is licensed under the GNU General Public License v3.0 - see the LICENSE file for details.

Full license text: https://www.gnu.org/licenses/gpl-3.0.en.html

## Contributing

1. Fork the repository
2. Create a feature branch (git checkout -b feature/amazing-feature)
3. Commit changes (git commit -m 'Add amazing feature')
4. Push to branch (git push origin feature/amazing-feature)
5. Open a Pull Request