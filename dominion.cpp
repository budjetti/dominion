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
        cards[CardId::FESTIVAL] = CardData(CardId::FESTIVAL, CardType::ACTION, "Festival", 5);
        cards[CardId::COUNCIL_ROOM] = CardData(CardId::COUNCIL_ROOM, CardType::ACTION, "Council Room", 5);
        cards[CardId::MARKET] = CardData(CardId::MARKET, CardType::ACTION, "Market", 5);
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

static bool GameShouldContinue(vector<vector<Card>> & shop){
    size_t emptyStacks = 0;
    for(vector<Card> v : shop){
        if(v.size() == 0)
            emptyStacks++;
    }

    if(emptyStacks == 0){
        // no stacks have depleted
        return true;
    } else if(emptyStacks >= 3){
        // at least 3 shop stacks have depleted
        return false;
    }

    for(vector<Card> v : shop){
        if(v.size() == 0)
            continue;
        if(v[0].data.id == CardId::PROVINCE){
            // 1-2 shop stacks have depleted but provinces remain
            return true;
        }
    }
    // provinces have depleted
    return false;
}

class Player{
public:
    Player(string name, vector<vector<Card>>* shop, vector<Card>* trash) : 
        name(name),
        shop(shop), 
        trash(trash), 
        gold(0), 
        autoClaim(true),
        debug(true) 
    {
        GainStartingCards();
        PopulateEffects();
        Draw(5);
    }
    
    void TakeTurn(vector<Player*> allPlayers){
        // find a better place for delivering allPlayers?
        players = allPlayers;
        if(debug){
            actions = 99;
            buys = 99;
            gold = 99;
        } else {
            actions = 1;
            buys = 1;
            gold = 0;
        }
        // only do buy phase if player did not end turn during the action phase
        if(PlayPhase(false)){
            PlayPhase(true);
        }
        EndTurn();
    }
    string GetName(){
        return name;
    }
    size_t Score(){
        size_t total = 0;
        size_t gardens = 0;
        vector<vector<Card>> deck{hand, discardPile, drawPile, playArea};
        for(vector<Card> v : deck){
            for(Card c : v){
                if(c.data.type != CardType::VICTORY)
                    continue;
                switch(c.data.id){
                    case CardId::ESTATE:
                        total++;
                        break;
                    case CardId::DUCHY:
                        total += 3;
                        break;
                    case CardId::PROVINCE:
                        total += 6;
                        break;
                    case CardId::CURSE:
                        total--;
                        break;
                    case CardId::GARDENS:
                        gardens++;
                        break;
                    default:
                        cout << "failed to score victory card\n";
                        break;
                }
            }
        }
        if(gardens > 0){
            size_t count = hand.size() + discardPile.size() + drawPile.size() + playArea.size();
            total += (count / 10) * gardens;
        }
        return total;
    }
private:
    // ------------------------------------------------ PRIVATE VARIABLES -------------------------------------------------------

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
    bool autoClaim;
    bool debug;
    
    // ------------------------------------------------ STRING -------------------------------------------------------

    string SubstrToCard(string alias, vector<Card> cards){
        vector<Card> unique;
        for(Card c : cards){
            bool isCopy = false;
            for(Card u : unique){
                if(u == c){
                    isCopy = true;
                    break;
                }
            }
            if(!isCopy){
                // is this copying necessary?
                Card copy = c;
                unique.push_back(copy);
            }
        }
        if(unique.size() == 0){
            // no matching results
            return "";
        }
        for(int i = 0; i < alias.size(); i++){
            vector<Card> matchingUnique = unique;
            for(Card c : unique){
                if(tolower(c.data.name[i]) != tolower(alias[i])){
                    vector<Card>::iterator pos = find(matchingUnique.begin(), matchingUnique.end(), c);
                    if(pos != matchingUnique.end())
                        matchingUnique.erase(pos);
                }
            }
            unique = matchingUnique;
            if(unique.size() == 1)
                // excactly one mathing result
                return unique[0].data.name;
        }
        // multiplte matching results
        return "";
    }
    // override meant specifically for shop
    string SubstrToCard(string alias, vector<vector<Card>> cardStacks){
        vector<Card> topCards;
        for(vector<Card> s : cardStacks){
            if(s.size() == 0)
                continue;
            Card copy = s[0];
            topCards.push_back(copy);
        }
        return SubstrToCard(alias, topCards);
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

    // ------------------------------------------------ SETUP -------------------------------------------------------

    void GainStartingCards(){
        if(debug){
            GainCard(CardId::COPPER, 1);
            GainCard(CardId::SILVER, 1);
            GainCard(CardId::SMITHY, 1);
            GainCard(CardId::CELLAR, 1);
            GainCard(CardId::LABORATORY, 1);
            GainCard(CardId::CHAPEL, 1);
            GainCard(CardId::REMODEL, 1);
            GainCard(CardId::WORKSHOP, 1);
            GainCard(CardId::LIBRARY, 1);
        } else {
            GainCard(CardId::COPPER, 7);
            GainCard(CardId::ESTATE, 3);
        }
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
        cardEffects[CardId::MOAT] = &Player::PlayMoat;
        cardEffects[CardId::WORKSHOP] = &Player::PlayWorkshop;
        cardEffects[CardId::MILITIA] = &Player::PlayMilitia;
        cardEffects[CardId::WITCH] = &Player::PlayWitch;
        cardEffects[CardId::SPY] = &Player::PlaySpy;
        cardEffects[CardId::COUNCIL_ROOM] = &Player::PlayCouncilRoom;
        cardEffects[CardId::ADVENTURER] = &Player::PlayAdventurer;
        cardEffects[CardId::THIEF] = &Player::PlayThief;
        cardEffects[CardId::CHANCELLOR] = &Player::PlayChancellor;
        cardEffects[CardId::REMODEL] = &Player::PlayRemodel;
        cardEffects[CardId::THRONE_ROOM] = &Player::PlayThroneRoom;
        cardEffects[CardId::FEAST] = &Player::PlayFeast;
        cardEffects[CardId::MARKET] = &Player::PlayMarket;
        cardEffects[CardId::BUREAUCRAT] = &Player::PlayBureaucrat;
        cardEffects[CardId::MINE] = &Player::PlayMine;
        cardEffects[CardId::MONEYLENDER] = &Player::PlayMoneylender;
        cardEffects[CardId::WOODCUTTER] = &Player::PlayWoodcutter;
        cardEffects[CardId::LIBRARY] = &Player::PlayLibrary;
    }

    // ------------------------------------------------ BASIC ACTIONS -------------------------------------------------------

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
                GainCard(name);
                cout << "Bought " << name << "\n";
                return true;
            }
        }
        // card not found in shop. maybe name was a substring?
        string retry = SubstrToCard(name, *shop);
        if(retry != name){
            return BuyCard(retry);
        }
        cout << "No card with unambiguously matcing name found in shop\n";
        return false;
    }
    // Gain a card from shop. Returns true if successful.
    bool GainCard(string name){
        for(auto &shopStack : *shop){
            if(shopStack.size() == 0){
                continue;
            }
            if(StrLower(shopStack.back().data.name) == StrLower(name)){
                discardPile.insert(discardPile.end(), make_move_iterator(shopStack.begin()), make_move_iterator(shopStack.begin() + 1));
                shopStack.erase(shopStack.begin(), shopStack.begin() + 1);
                return true;
            }
        }
        return false;
    }
    void ResolveEffect(CardId effect){
        if(cardEffects[effect]){
            // I love c++
            (this->*cardEffects[effect])();
        } else {
            cout << "Play function not found\n";
        }
    }
    // TODO card should be gained from shop, instead of being materialized into existence
    void GainCard(CardId id, int count){
        for(size_t i = 0; i < count; i++){
            Card card(id);
            discardPile.push_back(card);
            // cout << "added card " <<  discardPile.back().data.name << " to discard\n";
        }
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
    void MoveAllCards(vector<Card> & original, vector<Card> & destination){
        destination.insert(destination.end(), make_move_iterator(original.begin()), make_move_iterator(original.end()));
        original.erase(original.begin(), original.end());
    }

    // this function will break if cards have multiplte types
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

                cout << "Playing " << c.data.name << "...\n";
                ResolveEffect(c.data.id);
                return true;
            }
        }
        // card not found in hand. maybe it was a substring?
        string retry = SubstrToCard(name, hand);
        if(retry != name){
            return PlayCard(retry, type);
        }
        cout << "No card with unambiguously matcing name found in hand\n";
        return false;
    }
    void ClaimAll(){
        vector<string> treasures;
        for(Card c : hand){
            if(c.data.type == CardType::TREASURE)
                treasures.push_back(c.data.name);
        }
        for(string s : treasures)
            PlayCard(s, CardType::TREASURE);
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
    // ------------------------------------------------ CONTROL -------------------------------------------------------

    bool BasicCommands(vector<string> tokens){
        if(tokens.size() == 0){
            return false;
        } else if(tokens[0] == "shop" || tokens[0] == "s"){
            PrintShop();
            return true;
        } else if(tokens[0] == "status" || tokens[0] == "st"){
            PrintStatus(true);
            return true;
        } else if(tokens[0] == "discard" || tokens[0] == "di"){
            PrintDiscard();
            return true;
        } else if(tokens[0] == "hand" || tokens[0] == "h"){
            PrintHand();
            return true;
        } else if(tokens[0] == "played" || tokens[0] == "pd"){
            PrintPlayArea();
            return true;
        } else if(tokens[0] == "deck" || tokens[0] == "d"){
            PrintAll();
            return true;
        } else if(tokens[0] == "autoclaim" || tokens[0] == "ac"){
            ToggleAutoClaim(tokens.size() > 1 ? tokens[1] : "");
            return true;
        }
        return false;
    }
    void ToggleAutoClaim(string arg){
        if(arg == "on" || arg == "true"){
            autoClaim = true;
        } else if (arg == "off" || arg == "false"){
            autoClaim = false;
        } else {
            autoClaim = !autoClaim;
        }
        cout << "autoclaim is now " << (autoClaim ? "on" : "off") << "\n";
    }

    // Handles player's action/buy phase. Returns true if turn should continue or false if player wants to end their turn.
    bool PlayPhase(bool isBuyPhase){
        while(GameShouldContinue(*shop)){
            PrintStatus(true);
            cout << "[" << name << " - " << (isBuyPhase ? "buy" : "action") << "]: ";
            vector<string> tokens = ResponseToTokens();
            if(tokens.size() == 0){
                continue;
            }
            if(tokens.size() == 1){
                if(tokens[0] == "end" || tokens[0] == "e"){
                    return false;
                } else if(tokens[0] == "claim" || tokens[0] == "c"){
                    if(!isBuyPhase)
                        cout << "Move to buy phase\n";
                    ClaimAll();
                    if(isBuyPhase){
                        continue;
                    } else{
                        // move to buy phase
                        return true;
                    }
                }
            }
            if(BasicCommands(tokens)){
                continue;
            }
            if(tokens.size() > 1){
                if(tokens[0] == "play" || tokens[0] == "p"){
                    PlayCard(tokens[1], isBuyPhase ? CardType::TREASURE : CardType::ACTION);
                    continue;
                } else if (tokens[0] == "buy" || tokens[0] == "b"){
                    if(!isBuyPhase)
                        cout << "Move to buy phase\n";
                    if(autoClaim)
                        ClaimAll();
                    BuyCard(tokens[1]);
                    // check for game end

                    if(isBuyPhase){
                        continue;
                    } else{
                        // move to buy phase
                        return true;
                    }
                }
            }
            cout << "Invalid input\n";
        }
        return true;
    }
    void EndTurn(){
        MoveAllCards(hand, discardPile);
        MoveAllCards(playArea, discardPile);
        Draw(5);
    }

    // ------------------------------------------------ PRINTING -------------------------------------------------------

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
        PrintStatus(false);
        PrintPlayArea();
        PrintDrawPile();
        PrintDiscard();
    }
    void PrintStatus(bool includeHand){
        cout << "Gold: " << gold << " - Buys: " << buys << " - Actions: " << actions << (includeHand ? " - " : "\n");
        PrintHand();
    }
    void PrintHand(){
        cout << "Hand (" << hand.size() << "): ";
        PrintCardVector(hand);
    }
    void PrintDrawPile(){
        cout << "Draw (" << drawPile.size() << "): ";
        for(int i = 0; i < drawPile.size(); i++){
            cout << "?";
            if(i < drawPile.size() - 1){
                cout << ", ";
            }
        }
        cout << "\n";
    }
    void PrintPlayArea(){
        cout << "Play (" << playArea.size() << "): ";
        PrintCardVector(playArea);
    }
    void PrintDiscard(){
        cout << "Discard (" << discardPile.size() << "): ";
        PrintCardVector(discardPile);
    }
    void PrintShop(){
        CardType lastType = CardType::TREASURE;
        cout << "---------- SHOP ----------\n";
        for(auto shopStack : *shop){
            if(shopStack.size() == 0){
                cout << "Empty\n";
            } else {
                CardData data = shopStack.back().data;
                if(lastType != CardType::ACTION && data.type != lastType){
                    cout << "\n";
                    lastType = data.type;
                }
                cout << shopStack.size() << "x " << data.name << " (" << data.cost << ")\n";
            }
        }
        cout << "--------------------------\n";
    }

    // ------------------------------------------------ PLAY -------------------------------------------------------

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
        cout << "Discard (eg. estate estate copper / e e co / all): ";
        vector<string> tokens = ResponseToTokens();
        size_t discardedCount = 0;
        if(tokens[0] == "all"){
            // doing a for loop here seems to not discard the last card...
            while(hand.size() > 0){
                Discard(hand[0].data.id);
                discardedCount++;
            }
        } else {
            for(string name : tokens){
                name = SubstrToCard(name, hand);
                for(Card & card : hand){
                    if(StrLower(card.data.name) == StrLower(name)){
                        Discard(card.data.id);
                        discardedCount++;
                    }
                }
            }
        }
        Draw(discardedCount);
    }
    void PlayChapel(){
        cout << "Trash (eg. estate estate copper / e e co / all): ";
        vector<string> tokens = ResponseToTokens();
        if(tokens[0] == "all"){
            // doing a for loop here seems to not trash the last card...
            while(hand.size() > 0){
                Trash(hand[0].data.id);
            }
            return;
        }
        for(string name : tokens){
            for(Card & card : hand){
                if(StrLower(card.data.name) == StrLower(name)){
                    Trash(card.data.id);
                }
            }
        }
    }
    void PlayMoat(){
        Draw(2);
    }
    void PlayWorkshop(){
        cout << "Gain a card costing up to 4 (eg. gardens, village): ";
        vector<string> tokens = ResponseToTokens();
        if(tokens.size() == 0){
            cout << "Please input a card\n";
            PlayWorkshop();
            return;
        }
        string name = SubstrToCard(tokens[0], *shop);
        for(vector<Card> v : *shop){
            if(StrLower(v.back().data.name) == StrLower(name)){
                if(v.back().data.cost > 4){
                    cout << "Too expensive\n";
                    PlayWorkshop();
                    return;
                } else {
                    break;
                }
            }
        }
        if(GainCard(name)){
            cout << "Gained " << name << "\n";
        } else {
            cout << "Could not gain " << name << "\n";
        }
    }
    void PlayLibrary(){
        vector<Card> aside;
        while(hand.size() < 7 && (drawPile.size() > 0 || discardPile.size() > 0)){
            if(drawPile.size() == 0){
                ShuffleDiscardIntoDraw();
            }
            // TODO check if this should be front or back
            Card drawTop = drawPile.back();
            if(drawTop.data.type == CardType::ACTION){
                cout << "Set aside " << drawTop.data.name << "? (y/n): ";
                string answer;
                cin >> answer;
                if(StrLower(answer) == "y"){
                    vector<Card>::iterator pos = drawPile.end();
                    Card copy(drawTop.data.id);
                    aside.push_back(copy);
                    drawPile.erase(pos);
                    continue;
                }
            }
            // if not an action or action was not set aside
            Draw(1);
        }
        MoveAllCards(aside, discardPile);
    }
    void PlayWoodcutter(){
        buys++;
        gold += 2;
    }
    void PlayMoneylender(){
        if(Trash(CardId::COPPER)){
            cout << "Trashed Copper for 3 gold\n";
            gold += 3;
        } else {
            cout << "Failed to trash Copper\n";
        }
    }
    void PlayMine(){

    }
    void PlayBureaucrat(){

    }
    void PlayFeast(){

    }
    void PlayThroneRoom(){

    }
    void PlayMarket(){
        Draw(1);
        actions++;
        buys++;
        gold++;
    }
    void PlayRemodel(){
        if(hand.size() == 0){
            cout << "No card to trash.\n";
            return;
        }
        cout << "Trash card from hand (eg. silver / si): ";
        string response;
        cin >> response;
        string name = SubstrToCard(response, hand);
        for(Card c : hand){
            if(StrLower(c.data.name) == StrLower(name)){
                size_t newCardCost = c.data.cost + 2;
                cout << "Gain card with cost " << newCardCost << " or less (eg. copper): ";
                string secondResponse;
                cin >> secondResponse;
                string secondName = SubstrToCard(secondResponse, *shop);
                for(vector<Card> v : *shop){
                    if(v.back().data.name == secondName){
                        if(v.back().data.cost <= newCardCost){
                            GainCard(secondName);
                            cout << "Gained " << secondName << " \n";
                        } else {
                            cout << "Too expensive.\n";
                        }
                        break;
                    }
                }
                Trash(c.data.id);
                return;
            }
        }
        cout << "No card " << response << " found in hand.\n";
    }
    void PlayChancellor(){
        gold += 2;
        cout << "Move " << drawPile.size() << " cards from draw to discard? (y/n): ";
        string response;
        cin >> response;
        if(StrLower(response) == "y"){
            MoveAllCards(drawPile, discardPile);
            cout << "Cards moved.\n";
        }
    }
    void PlayThief(){

    }
    void PlayAdventurer(){

    }
    void PlayCouncilRoom(){

    }
    void PlaySpy(){

    }
    void PlayWitch(){

    }
    void PlayMilitia(){

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
        // debug
        victoryCount = 1;
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
        AddShopStack(CardId::GARDENS, 10);
        AddShopStack(CardId::MOAT, 10);
        AddShopStack(CardId::WORKSHOP, 10);
        AddShopStack(CardId::WOODCUTTER, 10);
        AddShopStack(CardId::MARKET, 10);
        AddShopStack(CardId::MONEYLENDER, 10);
        AddShopStack(CardId::MINE, 10);
        AddShopStack(CardId::BUREAUCRAT, 10);
        AddShopStack(CardId::FEAST, 10);
        AddShopStack(CardId::THRONE_ROOM, 10);
        AddShopStack(CardId::REMODEL, 10);
        AddShopStack(CardId::CHANCELLOR, 10);
        AddShopStack(CardId::LIBRARY, 10);
        AddShopStack(CardId::THIEF, 10);
        AddShopStack(CardId::ADVENTURER, 10);
        AddShopStack(CardId::COUNCIL_ROOM, 10);
        AddShopStack(CardId::SPY, 10);
        AddShopStack(CardId::WITCH, 10);
        AddShopStack(CardId::MILITIA, 10);

        while(PlayRound());

        EndGame();
    }
private:
    // returns true if game should continue
    bool PlayRound(){
        for(auto p : players){
            p->TakeTurn(players);
            cout << "---------- Turn ended ----------\n";
        }
        return GameShouldContinue(shop);
    }
    void EndGame(){
        cout << "Game has ended\n";
        // TODO sort by score
        for(Player * p : players){
            cout << p->GetName() << ": " << p->Score() << "\n";
        }
    }

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

int main(){
    Game g;
    g.Start();
}