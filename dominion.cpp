#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <map>
#include <sstream>
#include <cctype>

using namespace std;

enum class CardId{
    // TREASURE
    COPPER,
    SILVER,
    GOLD,
    // VICTORY
    ESTATE,
    DUCHY,
    PROVINCE,
    // CURSE
    CURSE,
    // ACTION (1st and 2nd edition)
    CELLAR,
    CHAPEL,
    MOAT,
    VILLAGE,
    WORKSHOP,
    BUREAUCRAT,
    GARDENS,
    MILITIA,
    MONEYLENDER,
    REMODEL,
    SMITHY,
    THRONE_ROOM,
    COUNCIL_ROOM,
    FESTIVAL,
    LABORATORY,
    LIBRARY,
    MARKET,
    MINE,
    WITCH,
    // ACTION (1st edition only)
    CHANCELLOR,
    WOODCUTTER,
    FEAST,
    SPY,
    THIEF,
    ADVENTURER,
    //ACTION (2nd edition only)
    HARBRINGER,
    MERCHANT,
    VASSAL,
    POACHER,
    BANDIT,
    SENTRY,
    ARTISAN,
    // OTHER
    NO_ID,
};
enum class CardType{
    ACTION,
    TREASURE,
    VICTORY,
    CURSE,
    NO_TYPE,
};
class CardData{
public:
    CardData(CardId _id, CardType _type, string _name, size_t _cost) : id(_id), type(_type), name(_name), cost(_cost){}
    CardData() : CardData(CardId::NO_ID, CardType::NO_TYPE, "No Name", 0){}
    CardId id;
    CardType type;
    string name;
    size_t cost;
};
class CardSet{
public:
    map<CardId, CardData> cards;
    CardSet(){
        // 1st EDITION CARDS (DEFAULT)

        // TREASURE
        cards[CardId::COPPER] = CardData(CardId::COPPER, CardType::TREASURE, "Copper", 0);
        cards[CardId::SILVER] = CardData(CardId::SILVER, CardType::TREASURE, "Silver", 3);
        cards[CardId::GOLD] = CardData(CardId::GOLD, CardType::TREASURE, "Gold", 6);
        // VICTORY
        cards[CardId::ESTATE] = CardData(CardId::ESTATE, CardType::VICTORY, "Estate", 2);
        cards[CardId::DUCHY] = CardData(CardId::DUCHY, CardType::VICTORY, "Duchy", 5);
        cards[CardId::PROVINCE] = CardData(CardId::PROVINCE, CardType::VICTORY, "Province", 8);
        cards[CardId::GARDENS] = CardData(CardId::GARDENS, CardType::VICTORY, "Gardens", 4);
        // CURSE
        cards[CardId::CURSE] = CardData(CardId::CURSE, CardType::CURSE, "Curse", 0);
        // ACTION
        cards[CardId::CELLAR] = CardData(CardId::CELLAR, CardType::ACTION, "Cellar", 2);
        cards[CardId::CHAPEL] = CardData(CardId::CHAPEL, CardType::ACTION, "Chapel", 2);
        cards[CardId::CHANCELLOR] = CardData(CardId::CHANCELLOR, CardType::ACTION, "Chancellor", 3);
        cards[CardId::VILLAGE] = CardData(CardId::VILLAGE, CardType::ACTION, "Village", 3);
        cards[CardId::WOODCUTTER] = CardData(CardId::WOODCUTTER, CardType::ACTION, "Woodcutter", 3);
        cards[CardId::WORKSHOP] = CardData(CardId::WORKSHOP, CardType::ACTION, "Workshop", 3);
        cards[CardId::FEAST] = CardData(CardId::FEAST, CardType::ACTION, "Feast", 4);
        cards[CardId::MONEYLENDER] = CardData(CardId::MONEYLENDER, CardType::ACTION, "Moneylender", 4);
        cards[CardId::REMODEL] = CardData(CardId::REMODEL, CardType::ACTION, "Remodel", 4);
        cards[CardId::SMITHY] = CardData(CardId::SMITHY, CardType::ACTION, "Smithy", 4);
        cards[CardId::THRONE_ROOM] = CardData(CardId::THRONE_ROOM, CardType::ACTION, "Throne Room", 4);
        cards[CardId::COUNCIL_ROOM] = CardData(CardId::COUNCIL_ROOM, CardType::ACTION, "Council Room", 5);
        cards[CardId::FESTIVAL] = CardData(CardId::FESTIVAL, CardType::ACTION, "Festival", 5);
        cards[CardId::LABORATORY] = CardData(CardId::LABORATORY, CardType::ACTION, "Laboratory", 5);
        cards[CardId::LIBRARY] = CardData(CardId::LIBRARY, CardType::ACTION, "Library", 5);
        cards[CardId::MINE] = CardData(CardId::MINE, CardType::ACTION, "Mine", 5);
        cards[CardId::ADVENTURER] = CardData(CardId::ADVENTURER, CardType::ACTION, "Adventurer", 6);
        // ACTION - REACTION
        cards[CardId::MOAT] = CardData(CardId::MOAT, CardType::ACTION, "Moat", 2);
        // ACTION - ATTACK
        cards[CardId::BUREAUCRAT] = CardData(CardId::BUREAUCRAT, CardType::ACTION, "Bureaucrat", 4);
        cards[CardId::MILITIA] = CardData(CardId::MILITIA, CardType::ACTION, "Militia", 4);
        cards[CardId::SPY] = CardData(CardId::SPY, CardType::ACTION, "Spy", 4);
        cards[CardId::THIEF] = CardData(CardId::THIEF, CardType::ACTION, "Thief", 4);
        cards[CardId::WITCH] = CardData(CardId::WITCH, CardType::ACTION, "Witch", 5);

        // TODO include 2nd edition?
    }
};
// !
static CardSet defaultCardSet;

class Card{
public:
    Card(CardId id) : data(defaultCardSet.cards[id]){} 
    // should be const ? ? ? ?
    bool operator==(Card other) const {
        return other.data.id == data.id;
    }
    CardData data;
};

class Player{
public:
    Player(string name, vector<vector<Card>>* shop, vector<Card> * trash) : name(name), shop(shop), trash(trash), gold(0) {
        cout << "New player created. Gaining starting cards. \n";
        GainStartingCards();
        PopulateEffects();
        Draw(5);
    }
    
    void TakeTurn(vector<Player*> allPlayers){
        // find a better place for delivering allPlayers?
        players = allPlayers;
        actions = 99;
        buys = 99;
        gold = 99;
        ActionPhase();
        BuyPhase();
        EndTurn();
    }
private:
    vector<Card> hand;
    vector<Card> discardPile;
    vector<Card> drawPile;
    vector<Card> playArea;
    vector<vector<Card>>* shop;
    vector<Card>* trash;
    vector<Player*> players;
    string name;
    size_t gold;
    size_t actions;
    size_t buys;
    map<CardId, void (Player::*)()> cardEffects;

    bool BasicCommands(vector<string> tokens){
        if(tokens.size() == 0){
            return false;
        } else if(tokens[0] == "shop"){
            PrintShop();
            return true;
        } else if(tokens[0] == "status"){
            PrintStatus();
            return true;
        } else if(tokens[0] == "discard"){
            PrintDiscard();
            return true;
        } else if(tokens[0] == "hand"){
            PrintHand();
            return true;
        } else if(tokens[0] == "played"){
            PrintPlayArea();
            return true;
        } else if(tokens[0] == "all"){
            PrintAll();
            return true;
        }
        return false;
    }
    void ActionPhase(){
        while(1){
            PrintStatus();
            cout << "[" << name << " - action]: ";
            vector<string> tokens = ResponseToTokens();
            if(tokens.size() == 0){
                continue;
            } else if(tokens.size() == 1 && tokens[0] == "end"){
                break;
            } else if(BasicCommands(tokens)){
                continue;
            } else if (tokens.size() > 1){
                if(tokens[0] == "play"){
                    PlayCard(tokens[1], CardType::ACTION);
                    continue;
                } else if (tokens[0] == "buy"){
                    cout << "Can't buy during the action phase\n";
                    continue;
                }
            }
            cout << "Invalid input\n";
        }
    }
    void BuyPhase(){
        while(1){
            PrintStatus();
            cout << "[" << name << " - buy]: ";
            vector<string> tokens = ResponseToTokens();
            if(tokens.size() == 0){
                continue;
            } else if(tokens.size() == 1 && tokens[0] == "end"){
                break;
            } else if(BasicCommands(tokens)){
                continue;
            } else if (tokens.size() > 1){
                if(tokens[0] == "play"){
                    PlayCard(tokens[1], CardType::TREASURE);
                    continue;
                } else if (tokens[0] == "buy"){
                    BuyCard(tokens[1]);
                    continue;
                }
            }
            cout << "Invalid input\n";
        }
    }
    string StrLower(string original){
        transform(original.begin(), original.end(), original.begin(), [](unsigned char c){return tolower(c);});
        return original;
    }
    vector<string> ResponseToTokens(){
        string response;
        getline(cin, response);
        return TokenizeString(response);
    }
    vector<string> TokenizeString(string original){
        vector<string> tokens;
        string lower = StrLower(original);
        istringstream iss(lower);
        string s;
        while(getline(iss, s, ' ')){
            tokens.push_back(s.c_str());
        }
        return tokens;
    }
    void GainStartingCards(){
        GainCard(CardId::COPPER, 1);
        GainCard(CardId::SILVER, 1);
        GainCard(CardId::SMITHY, 1);
        GainCard(CardId::CELLAR, 1);
        GainCard(CardId::LABORATORY, 1);
        GainCard(CardId::CHAPEL, 1);
    }
    void PopulateEffects(){
        cardEffects[CardId::COPPER] = &Player::PlayCopper;
        cardEffects[CardId::SILVER] = &Player::PlaySilver;
        cardEffects[CardId::GOLD] = &Player::PlayGold;
        cardEffects[CardId::SMITHY] = &Player::PlaySmithy;
        cardEffects[CardId::VILLAGE] = &Player::PlayVillage;
        cardEffects[CardId::FESTIVAL] = &Player::PlayFestival;
        cardEffects[CardId::LABORATORY] = &Player::PlayLaboratory;
        cardEffects[CardId::CELLAR] = &Player::PlayCellar;
        cardEffects[CardId::CHAPEL] = &Player::PlayChapel;
    }
    bool BuyCard(string name){
        if(buys <= 0){
            cout << "No buys remaining\n";
            return false;
        }
        for(auto &shopStack : *shop){
            if(shopStack.size() == 0)
                continue;
            if(StrLower(shopStack.back().data.name) == StrLower(name)){
                if(gold < shopStack.back().data.cost){
                    cout << "Not enough gold\n";
                    return false;
                }
                buys--;
                gold -= shopStack.back().data.cost;
                // move card from shop to discard
                discardPile.insert(discardPile.end(), make_move_iterator(shopStack.begin()), make_move_iterator(shopStack.begin() + 1));
                shopStack.erase(shopStack.begin(), shopStack.begin() + 1);
                cout << "Bought " << name << "\n";
                return true;
            }
        }
        cout << "No " << name << " to buy\n";
        return false;
    }
    void PrintShop(){
        for(auto shopStack : *shop){
            if(shopStack.size() == 0){
                cout << "Empty stack\n";
                continue;
            }
            cout << shopStack.back().data.name << " (" << shopStack.size() << ")\n";
        }
    }
    void ResolveEffect(CardId effect){
        if(cardEffects[effect]){
            // I love c++
            (this->*cardEffects[effect])();
        } else {
            cout << "Play function not found\n";
        }
    }
    void GainCard(CardId id, int count){
        for(size_t i = 0; i < count; i++){
            Card card(id);
            discardPile.push_back(card);
            // cout << "added card " <<  discardPile.back().data.name << " to discard\n";
        }
    }
    void PrintCardVector(vector<Card> vec){
        for(int i = 0; i < vec.size(); i++){
            cout << vec[i].data.name;
            if(i < vec.size() - 1){
                cout << ", ";
            }
        }
        cout << "\n";
    }
    void PrintAll(){
        PrintStatus();
        PrintPlayArea();
        PrintDiscard();
    }
    void PrintStatus(){
        cout << "Gold: " << gold << " - Buys: " << buys << " - Actions: " << actions << " - ";
        PrintHand();
    }
    void PrintHand(){
        cout << "Hand: ";
        PrintCardVector(hand);
    }
    void PrintPlayArea(){
        cout << "Play Area: ";
        PrintCardVector(playArea);
    }
    void PrintDiscard(){
        cout << "Discard: ";
        PrintCardVector(discardPile);
    }
    // this function works, but it's written in a goofy way
    bool MoveCard(CardId id, vector<Card> & oldVec, vector<Card> & newVec){
        for(Card c : oldVec){
            if(c.data.id == id){
                vector<Card>::iterator pos = find(oldVec.begin(), oldVec.end(), c);
                Card copy(c.data.id);
                newVec.push_back(copy);
                oldVec.erase(pos);
                return true;
            }
        }
        return false;
    }
    bool Trash(CardId id){
        return MoveCard(id, hand, *trash);
    }
    bool Discard(CardId id){
        return MoveCard(id, hand, discardPile);
    }
    bool Draw(int count){
        for(size_t i = 0; i < count; i++){
            if(drawPile.size() > 0){
                // drawPile.back() is the topmost card
                // cout << "drawing 1 card\n";
                Card card = drawPile.back();
                hand.push_back(card);
                drawPile.pop_back();
                // cout << "draw pile now has a size of " << drawPile.size() << "\n";
            } else {
                if(discardPile.size() > 0){
                    ShuffleDiscardIntoDraw();
                    Draw(1);
                } else {
                    // cout << "both draw and discard are empty, no cards to draw\n";
                    return false;
                }
            }
        }
        return true;
    }
    void EndTurn(){
        // move entirety of hand and play area into discard
        discardPile.insert(discardPile.end(), make_move_iterator(hand.begin()), make_move_iterator(hand.end()));
        hand.erase(hand.begin(), hand.end());
        discardPile.insert(discardPile.end(), make_move_iterator(playArea.begin()), make_move_iterator(playArea.end()));
        playArea.erase(playArea.begin(), playArea.end());

        Draw(5);
    }

    bool PlayCard(string name, CardType type){
        for(Card c : hand){
            if(StrLower(c.data.name) == StrLower(name)){
                // -------------- FAIL CONDITIONS -------------- 
                if(actions <= 0 && type == CardType::ACTION){
                    cout << "No actions remaining.\n";
                    return false;
                }
                if(c.data.type == CardType::ACTION){
                    if(type == CardType::TREASURE){
                        cout << c.data.name << " is not a treasure card\n";
                        return false;
                    }
                } else if(c.data.type == CardType::TREASURE && type == CardType::ACTION){
                    cout << c.data.name << " is not an action card\n";
                    return false;
                } else if(c.data.type == CardType::VICTORY){
                    cout << c.data.name << " is unplayable\n";
                    return false;
                }
                // -------------- SUCCESSFUL PLAY -------------- 
                if(c.data.type == CardType::ACTION){
                    actions--;
                }
                // move card to play area
                playArea.push_back(c);
                vector<Card>::iterator pos = find(hand.begin(), hand.end(), c);
                if(pos != hand.end()){
                    hand.erase(pos);
                }

                ResolveEffect(c.data.id);
                cout << "Played " << c.data.name << "\n";
                return true;
            }
        }
        cout << name << " is not a card in hand\n";
        return false;
    }

    void ShuffleDiscardIntoDraw(){
        // cout << "Shuffling discard into draw\n";
        // move cards from discard to draw
        // there's probably a better way to do this
        for(const auto & c : discardPile){
            Card card = c;
            drawPile.push_back(card);
        }
        while(discardPile.size() > 0){
            discardPile.pop_back();
        }
        unsigned seed = chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(drawPile.begin(), drawPile.end(), default_random_engine(seed));
        // cout << "draw pile now has a size of " << drawPile.size() << "\n";
    }
    void PlayCopper(){
        gold++;
        // cout << "Played Copper for 1 gold\n";
    }
    void PlaySilver(){
        gold += 2;
        // cout << "Played Silver for 2 gold\n";
    }
    void PlayGold(){
        gold += 3;
        // cout << "Played Gold for 3 gold\n";
    }
    void PlaySmithy(){
        Draw(3);
        // cout << "Played Smithy to draw 3 cards\n";
    }
    void PlayVillage(){
        Draw(1);
        actions += 2;
    }
    void PlayFestival(){
        actions += 2;
        buys++;
        gold += 2;
    }
    void PlayLaboratory(){
        Draw(2);
        actions++;
    }
    void PlayCellar(){
        actions++;
        cout << "Discard (eg. estate estate copper): ";
        vector<string> tokens = ResponseToTokens();
        size_t discardedCount = 0;
        for(string name : tokens){
            for(Card & card : hand){
                if(StrLower(card.data.name) == StrLower(name)){
                    Discard(card.data.id);
                    discardedCount++;
                }
            }
        }
        Draw(discardedCount);
    }
    void PlayChapel(){
        cout << "Trash (eg. estate estate copper): ";
        vector<string> tokens = ResponseToTokens();
        for(string name : tokens){
            for(Card & card : hand){
                if(StrLower(card.data.name) == StrLower(name)){
                    Trash(card.data.id);
                }
            }
        }
    }
};

class Game{
public:
    Game(){}
    void Start(){
        // I wonder if this can cause problems in the future if player goes out of scope or something 
        Player playerOne("Player One", &shop, &trash);
        players.push_back(&playerOne);

        // create shop
        size_t victoryCount = players.size() > 2 ? 12 : 8;
        size_t curseCount = players.size() > 1 ? (players.size() - 1) * 10 : 10;
        size_t copperCount = 60 - players.size() * 7;
        // BASIC CARDS
        AddShopStack(CardId::COPPER, copperCount);
        AddShopStack(CardId::SILVER, 40);
        AddShopStack(CardId::GOLD, 30);
        AddShopStack(CardId::ESTATE, victoryCount);
        AddShopStack(CardId::DUCHY, victoryCount);
        AddShopStack(CardId::PROVINCE, victoryCount);
        AddShopStack(CardId::CURSE, 20);
        // SELECTED CARDS
        AddShopStack(CardId::SMITHY, 10);
        AddShopStack(CardId::VILLAGE, 10);
        AddShopStack(CardId::FESTIVAL, 10);
        AddShopStack(CardId::LABORATORY, 10);
        AddShopStack(CardId::CELLAR, 10);
        AddShopStack(CardId::CHAPEL, 10);

        while(PlayRound());

        EndGame();
    }
private:
    // returns true if game should continue
    bool PlayRound(){
        // cout << "playing round" << endl;
        for(auto p : players){
            p->TakeTurn(players);
            cout << "turn finished\n";
        }
        return true;
    }
    void EndGame(){}

    void AddShopStack(CardId id, int count){
        vector<Card> cards;
        for(size_t i = 0; i < count; i++){
            Card card(id);
            cards.push_back(card);
        }
        shop.push_back(cards);
    }

    vector<Player*> players;
    vector<vector<Card>> shop;
    vector<Card> trash;
};

void TestStuff(){
    Game g;
    g.Start();
}
int main(){
    TestStuff();
}