/*
FOSS command line adaptation of Dominion (2008) written in c++

This is a hobby project. I am not affiliated with Rio Grande Games and I do not intend to
make money with this project.

Yes, I am mentally deranged, have brain rot and use phrases like "goober", "amogus" and "skibidi" in everyday conversation.

Dominion Wiki
https://wiki.dominionstrategy.com/index.php/Dominion_(Base_Set)

Repo: github.com/budjetti/dominion
Author: budjetti
*/

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
#include <optional>
#include <stdexcept>

// Controversial line of code ahead
using namespace std;

static const string VERSION_NUMBER = "1.0.3";

/*
List of all card ID's. Used alongside CardData.name to identify cards. Player uses CardId's to determine which
card effect to resolve when playing a card.
*/
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
    // OTHER
    NO_ID,
};

/*
List of all card types. Useful in certain operations.
*/
enum class CardType{
    ACTION,
    TREASURE,
    VICTORY,
    CURSE,
    NO_TYPE,
};

/*
Contains everything that a player needs to know about a card: id, type, name and cost. CardData does not
contain instructions on how to resolve the cards effect. That is handled by the player based on CardData.id.
*/
class CardData{
public:
    CardData(CardId _id, CardType _type, string _name, size_t _cost, string _description) : id(_id), type(_type), name(_name), cost(_cost), description(_description){}
    CardData() : CardData(CardId::NO_ID, CardType::NO_TYPE, "No Name", 0, "No Description"){}
    CardId id;
    CardType type;
    string name;
    size_t cost;
    string description;
};

/*
Initializes CardData for each card.
*/
class CardSet{
public:
    map<CardId, CardData> cards;
    CardSet(){
        // 1st EDITION CARDS (DEFAULT)

        // TREASURE
        cards[CardId::COPPER] = CardData(CardId::COPPER, CardType::TREASURE, "Copper", 0, "+1 Coin.");
        cards[CardId::SILVER] = CardData(CardId::SILVER, CardType::TREASURE, "Silver", 3, "+2 Coins.");
        cards[CardId::GOLD] = CardData(CardId::GOLD, CardType::TREASURE, "Gold", 6, "+3 Coins.");
        // VICTORY
        cards[CardId::ESTATE] = CardData(CardId::ESTATE, CardType::VICTORY, "Estate", 2, "Worth 1 victory point.");
        cards[CardId::DUCHY] = CardData(CardId::DUCHY, CardType::VICTORY, "Duchy", 5, "Worth 3 victory points.");
        cards[CardId::PROVINCE] = CardData(CardId::PROVINCE, CardType::VICTORY, "Province", 8, "Worth 6 victory points.");
        cards[CardId::GARDENS] = CardData(CardId::GARDENS, CardType::VICTORY, "Gardens", 4, "Worth 1 victory point per 10 cards you have (round down).");
        // CURSE
        cards[CardId::CURSE] = CardData(CardId::CURSE, CardType::CURSE, "Curse", 0, "Worth -1 victory point.");
        // ACTION
        cards[CardId::CELLAR] = CardData(CardId::CELLAR, CardType::ACTION, "Cellar", 2, "+1 Action. Discard any number of cards, then draw that many.");
        cards[CardId::CHAPEL] = CardData(CardId::CHAPEL, CardType::ACTION, "Chapel", 2, "Trash up to 4 cards from your hand.");
        cards[CardId::CHANCELLOR] = CardData(CardId::CHANCELLOR, CardType::ACTION, "Chancellor", 3, "+2 Coins. You may immediately put your deck into your discard pile.");
        cards[CardId::VILLAGE] = CardData(CardId::VILLAGE, CardType::ACTION, "Village", 3, "+1 Card. +2 Actions.");
        cards[CardId::WOODCUTTER] = CardData(CardId::WOODCUTTER, CardType::ACTION, "Woodcutter", 3, "+2 Coins. +1 Buy.");
        cards[CardId::WORKSHOP] = CardData(CardId::WORKSHOP, CardType::ACTION, "Workshop", 3, "Gain a card costing up to 4 coins.");
        cards[CardId::FEAST] = CardData(CardId::FEAST, CardType::ACTION, "Feast", 4, "Trash this card. Gain a card costing up to 5 Coins.");
        cards[CardId::MONEYLENDER] = CardData(CardId::MONEYLENDER, CardType::ACTION, "Moneylender", 4, "You may trash a Copper from your hand for +3 Coins.");
        cards[CardId::REMODEL] = CardData(CardId::REMODEL, CardType::ACTION, "Remodel", 4, "Trash a card from your hand. Gain a card costing up to 2 coins more than it.");
        cards[CardId::SMITHY] = CardData(CardId::SMITHY, CardType::ACTION, "Smithy", 4, "+3 Cards.");
        cards[CardId::THRONE_ROOM] = CardData(CardId::THRONE_ROOM, CardType::ACTION, "Throne Room", 4, "You may play an action card from your hand twice.");
        cards[CardId::FESTIVAL] = CardData(CardId::FESTIVAL, CardType::ACTION, "Festival", 5, "+2 Actions. +1 Buy. +2 Coins.");
        cards[CardId::COUNCIL_ROOM] = CardData(CardId::COUNCIL_ROOM, CardType::ACTION, "Council Room", 5, "+4 Cards. +1 Buy. Each other player draws a card.");
        cards[CardId::MARKET] = CardData(CardId::MARKET, CardType::ACTION, "Market", 5, "+1 Card. +1 Action. +1 Buy. +1 Coin.");
        cards[CardId::LABORATORY] = CardData(CardId::LABORATORY, CardType::ACTION, "Laboratory", 5, "+2 Cards. +1 Action.");
        cards[CardId::LIBRARY] = CardData(CardId::LIBRARY, CardType::ACTION, "Library", 5, "Draw until you have 7 cards in hand, skipping any Action cards you choose to; set those aside, discarding them afterwards.");
        cards[CardId::MINE] = CardData(CardId::MINE, CardType::ACTION, "Mine", 5, "You may trash a Treasure from your hand. Gain a Treasure to your hand costing up to 3 Coins more than it.");
        cards[CardId::ADVENTURER] = CardData(CardId::ADVENTURER, CardType::ACTION, "Adventurer", 6, "Reveal cards from your deck until you reveal 2 Treasure cards. Put those Treasure cards into your hand and discard the other revealed cards.");
        // ACTION - REACTION
        cards[CardId::MOAT] = CardData(CardId::MOAT, CardType::ACTION, "Moat", 2, "+2 Cards. | When another player plays an attack cards, you may first reveal this from your hand, to be unaffected by it.");
        // ACTION - ATTACK
        cards[CardId::BUREAUCRAT] = CardData(CardId::BUREAUCRAT, CardType::ACTION, "Bureaucrat", 4, "Gain a Silver onto your deck. Each other player reveals a Victory cards from their hand and puts it onto their deck (or reveals a hand with no Victory cards).");
        cards[CardId::MILITIA] = CardData(CardId::MILITIA, CardType::ACTION, "Militia", 4, "+2 Coins. Each other player discards down to 3 cards in hand.");
        cards[CardId::SPY] = CardData(CardId::SPY, CardType::ACTION, "Spy", 4, "+1 Card. +1 Action. Each player (including you) reveals the top card of their deck and either discards it or puts it back, your choice.");
        cards[CardId::THIEF] = CardData(CardId::THIEF, CardType::ACTION, "Thief", 4, "Each other player reveals the top 2 cards of their deck. If they reveal any Treasure cards, they trash one of them that you choose. You may gain any or all of these trashed cards. They discard the other revealed cards.");

        cards[CardId::WITCH] = CardData(CardId::WITCH, CardType::ACTION, "Witch", 5, "+2 Cards. Each other player gains a curse.");
        // NONE
        cards[CardId::NO_ID] = CardData(CardId::NO_ID, CardType::NO_TYPE, "No Name", 0, "No Description");

        // TODO include 2nd edition?
    }
};

// !
// TODO find a better place for this line?
static CardSet defaultCardSet;

/*
A card, containing data. Supports == operator.
*/
class Card{
public:
    Card(CardId id) : data(defaultCardSet.cards[id]){}
    // should be const ? ? ? ?
    bool operator==(Card other) const {
        return other.data.id == data.id;
    }
    CardData data;
};

// ------------------------------------------ STATIC FUNCTIONS ------------------------------------

static size_t EmptySupplyPileCount(vector<vector<Card>> & shop){
    size_t emptyStacks = 0;
    for(vector<Card> v : shop){
        if(v.size() == 0)
            emptyStacks++;
    }
    return emptyStacks;
}

/*
Checks game end conditions and returns false if they have been met.
*/
static bool GameShouldContinue(vector<vector<Card>> & shop){
    size_t emptyStacks = EmptySupplyPileCount(shop);
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

/*
For example, CardId::ESTATE -> "Estate"
*/
static const CardData IdToCardData(CardId id){
    Card card(id);
    return card.data;
}

/*
Given a substring and a vector<Card>, finds card in said vector whose name most
resembles substring, and returns a copy of the card.
*/
static const Card FindCard(string alias, vector<Card> cards){
    // make a copy of cards with no duplicates
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
        // searched card pile was empty
        return Card(CardId::NO_ID);
    }
    for(int i = 0; i < alias.length(); i++){
        vector<Card> matchingUnique = unique;
        for(Card c : unique){
            if(tolower(c.data.name[i]) != tolower(alias[i])){
                vector<Card>::iterator pos = find(matchingUnique.begin(), matchingUnique.end(), c);
                if(pos != matchingUnique.end())
                    matchingUnique.erase(pos);
            }
        }
        unique = matchingUnique;
    }
    // success - excatly one matching result
    if(unique.size() == 1)
        return unique[0];

    // multiple cards with different results match search term - result is ambiguous
    return Card(CardId::NO_ID);
}
/*
Given a substring and a vector<vector<Card>> (shop), finds card in shop whose name most
resembles substring, and returns a copy of that card.
*/
static const Card FindCard(string alias, vector<vector<Card>> cardStacks){
    vector<Card> topCards;
    for(vector<Card> s : cardStacks){
        if(s.size() == 0)
            continue;
        Card copy = s[0];
        topCards.push_back(copy);
    }
    return FindCard(alias, topCards);
}

static const vector<Card> FindCardsOfType(CardType type, vector<Card> cards){
    vector<Card> found;
    for(Card c : cards){
        if(c.data.type == type){
            found.push_back(c);
        }
    }
    return found;
}

/*
SRY CAPS
*/
static const string StrLower(string original){
    transform(original.begin(), original.end(), original.begin(), [](unsigned char c){return tolower(c);});
    return original;
}

/*
Takes words from string, lowercases them, and puts them in a vector.
*/
static const vector<string> TokenizeString(string original){
    vector<string> tokens;
    string lower = StrLower(original);
    istringstream iss(lower);
    string s;
    while(getline(iss, s, ' ')){
        tokens.push_back(s.c_str());
    }
    return tokens;
}

/*
Gets input with cin, takes words from input, lowercases them, and puts them in a vector.
*/
static const vector<string> ResponseToTokens(string prompt){
    cout << prompt;
    string response;
    getline(cin, response);
    return TokenizeString(response);
}

/*
Moves all cards from one vector to another.
*/
static const void MoveAllCards(vector<Card> & original, vector<Card> & destination){
    destination.insert(destination.end(), make_move_iterator(original.begin()), make_move_iterator(original.end()));
    original.erase(original.begin(), original.end());
}

/*
Use after prompting user to answer (y/n)
*/
static const bool Confirm(optional<string> defaultResponse = nullopt){
    vector<string> response = ResponseToTokens("");
    if(response.size() == 0){
        if(defaultResponse)
            return (defaultResponse == "y" || defaultResponse == "Y");
        else
            return Confirm();
    }

    if (response[0] == "y" || response[0] == "Y")
        return true;
    else if (response[0] == "n" || response[0] == "N")
        return false;
    else
        return Confirm();
}

// !
static const bool debug = false;

/*
Responsible for most actions in the game, like taking turns and playing cards.
*/
class Player{
public:
    Player(string name, vector<vector<Card>>* shop, vector<Card>* trash, vector<Player*>* allPlayers) :
        name(name),
        shop(shop),
        trash(trash),
        allPlayers(allPlayers),
        gold(0),
        autoClaim(true),
        turnCount(0),
        autoResponse(false)
    {
        GainStartingCards();
        PopulateEffects();
        Draw(5);
    }

    /*
    For people who are bad at names.
    */
    string GetName(){
        return name;
    }

    /*
    Resolve action, buy and end phases.
    TODO deliver allPlayers somewhere else
    */
    virtual void TakeTurn(){
        turnCount++;
        if(debug){
            actions = 99;
            buys = 99;
            gold = 99;
        } else {
            actions = 1;
            buys = 1;
            gold = 0;
        }

        // action phase
        if(PlayPhase(false)){
            // buy phase - skipped if player wants to end their turn
            PlayPhase(true);
        }
        // end phase
        EndTurn();
    }

    /*
    Tallies victory points.
    */
    size_t Score(){
        size_t total = 0;
        size_t gardens = 0;
        for(Card c : GetDeck()){
            if(!(c.data.type == CardType::VICTORY || c.data.type == CardType::CURSE))
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
        if(gardens > 0){
            size_t count = hand.size() + discardPile.size() + drawPile.size() + playArea.size();
            total += (count / 10) * gardens;
        }
        return total;
    }

    void PrintScore(){
        cout << "- " << name << " -\n";
        cout << "Score: " << Score() << "\n";
        cout << "Turns: " << turnCount << "\n";
        PrintDeck();
        cout << "\n";
    }
protected:
    // ------------------------------------------------ MEMBER VARIABLES -------------------------------------------------------

    vector<Card> hand;
    vector<Card> discardPile;
    vector<Card> drawPile;
    vector<Card> playArea;
    vector<vector<Card>>* shop;
    vector<Card>* trash;
    vector<Player*>* allPlayers;
    string name;
    size_t gold;
    size_t actions;
    size_t buys;
    map<CardId, bool (Player::*)()> cardEffects;
    bool autoClaim;
    size_t turnCount;
    bool autoResponse; // mainly for bots

    // ------------------------------------------------ SETUP -------------------------------------------------------

    /*
    Gain 7 Copper and 3 Estates, or something else if debug mode is on.
    */
    void GainStartingCards(){
        // TODO make sure these cards are not taken from shop, or at least account for it
        if(debug){
            CreateAndGainCard(CardId::MINE, 2);
        } else {
            CreateAndGainCard(CardId::COPPER, 7);
            CreateAndGainCard(CardId::ESTATE, 3);
        }
    }

    /*
    Makes sure CardId's are hooked to their respective effects.
    */
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

    /*
    Finds cards by name OR substring and buys it.
    */
    bool BuyCard(string cardName, optional<bool> verbose = true){
        if(buys <= 0){
            if(*verbose)
                cout << "No buys remaining\n";
            return false;
        }
        for(auto &shopStack : *shop){
            if(shopStack.size() == 0)
                continue;
            if(StrLower(shopStack.back().data.name) == StrLower(cardName)){
                if(gold < shopStack.back().data.cost){
                    if(*verbose)
                        cout << "Not enough gold\n";
                    return false;
                }
                buys--;
                gold -= shopStack.back().data.cost;
                GainCard(cardName);
                cout << "Bought " << cardName << "\n";
                return true;
            }
        }
        // card not found in shop. maybe name was a substring?
        string retry = FindCard(cardName, *shop).data.name;
        if(retry != cardName){
            return BuyCard(retry, *verbose);
        }
        if(*verbose)
            cout << "No card with unambiguously matcing name found in shop.\n";
        return false;
    }


    /*
    Retreive card from shop. Goes into discard if destination is not specified.
    */
    bool GainCard(string name, optional<vector<Card>*> destinationParam = nullopt){
        vector<Card> * destination = &discardPile;
        if(destinationParam){
            destination = *destinationParam;
        }
        for(auto &shopStack : *shop){
            if(shopStack.size() == 0){
                continue;
            }
            if(StrLower(shopStack.back().data.name) == StrLower(name)){
                destination->insert(destination->end(), make_move_iterator(shopStack.begin()), make_move_iterator(shopStack.begin() + 1));
                shopStack.erase(shopStack.begin(), shopStack.begin() + 1);
                return true;
            }
        }
        return false;
    }

    /*
    Spawn a card into discard out of thin air.
    */
    void CreateAndGainCard(CardId id, int count){
        for(size_t i = 0; i < count; i++){
            Card card(id);
            discardPile.push_back(card);
        }
    }

    /*
    Executes on-play effects of a card by its id. If card is played incorrectly, retry.
    */
    void ResolveEffect(CardId effect){
        if(cardEffects[effect]){
            // I love c++
            bool success = (this->*cardEffects[effect])();
            if(!success){
                ResolveEffect(effect);
            }
        } else {
            cout << "Play function not found\n";
        }
    }

    /*
    Moves a (random) card with a matching id from one vector to another. If ID is not provided, moves top card.
    */
    bool MoveCard(vector<Card> & oldVec, vector<Card> & newVec, optional<CardId> targetId = nullopt){
        if(oldVec.size() == 0){
            cout << "empty\n";
            return false;
        }
        CardId id = CardId::NO_ID;
        if(targetId){
            id = *targetId;
        }
        if(id == CardId::NO_ID){
            vector<Card>::iterator pos = drawPile.end();
            Card copy(oldVec.back().data.id);
            newVec.push_back(copy);
            drawPile.erase(pos);
            return true;
        }

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

    /*
    Moves a (random) card with a matching id to trash. Searches hand by default, draw pile if second param is true (thief)
    */
    bool Trash(CardId id, optional<bool> fromDraw = false){
        if(*fromDraw){
            return MoveCard(drawPile, *trash, id);
        }
        return MoveCard(hand, *trash, id);
    }

    /*
    Moves a (random) card with a matching id to discard. Searches hand by default, draw pile if second param is true (spy)
    */
    bool Discard(CardId id, optional<bool> fromDraw = false){
        if(*fromDraw){
            return MoveCard(drawPile, discardPile, id);
        }
        return MoveCard(hand, discardPile, id);
    }

    /*
    Shuffling randomness is based on system clock.
    */
    void ShuffleDiscardIntoDraw(){
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
    }

    /*
    Draws specified amount of cards, shuffling discard into draw as appropriate.
    */
    bool Draw(int count){
        for(size_t i = 0; i < count; i++){
            if(drawPile.size() > 0){
                // drawPile.back() is the topmost card
                Card card = drawPile.back();
                hand.push_back(card);
                drawPile.pop_back();
            } else {
                if(discardPile.size() > 0){
                    ShuffleDiscardIntoDraw();
                    Draw(1);
                } else {
                    // both draw and discard are empty, no cards to draw
                    return false;
                }
            }
        }
        return true;
    }

    /*
    Play all treasure cards in hand and move to buy phase.
    */
    void ClaimAll(){
        // Do not try to do this in one for loop. It will break.
        vector<string> treasures;
        for(Card c : hand){
            if(c.data.type == CardType::TREASURE){
                treasures.push_back(c.data.name);
            }
        }
        for(string s : treasures){
            PlayCard(s, CardType::TREASURE);
        }
    }

    const vector<Card> GetDeck(){
        vector<vector<Card>> piles{hand, discardPile, drawPile, playArea};
        vector<Card> deck;
        deck.reserve(hand.size() + discardPile.size() + drawPile.size() + playArea.size());
        for(auto v : piles){
            deck.insert(deck.end(), v.begin(), v.end());
        }
        return deck;
    }

    /*
    Finds a card of matching type in hand by name OR substring and attemps to play it, printing erorrs as appropriate.
    */
    bool PlayCard(string name, CardType type, optional<bool> verbose = true){
        for(Card c : hand){
            if(StrLower(c.data.name) == StrLower(name)){
                // -------------- FAIL CONDITIONS --------------
                if(actions <= 0 && type == CardType::ACTION){
                    if(*verbose)
                        cout << "No actions remaining.\n";
                    return false;
                }
                if(c.data.type == CardType::ACTION){
                    if(type == CardType::TREASURE){
                        if(*verbose)
                            cout << c.data.name << " is not a treasure card\n";
                        return false;
                    }
                } else if(c.data.type == CardType::TREASURE && type == CardType::ACTION){
                    if(*verbose)
                        cout << c.data.name << " is not an action card\n";
                    return false;
                } else if(c.data.type == CardType::VICTORY){
                    if(*verbose)
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
        string retry = FindCard(name, hand).data.name;
        if(retry != name){
            return PlayCard(retry, type, *verbose);
        }
        if(*verbose){
            cout << "No card with unambiguously matcing name found in hand\n";
        }
        return false;
    }

    // ------------------------------------------------ CONTROL -------------------------------------------------------

    /*
    Recognize and execute commands accessible during both the buy and action phase.
    */
    bool BasicCommands(vector<string> tokens){
        if(tokens.size() == 0){
            return false;
        } else if(tokens[0] == "shop" || tokens[0] == "s"){
            PrintShop();
            return true;
        // } else if(tokens[0] == "status" || tokens[0] == "st"){
        //     PrintStatus(true);
        //     return true;
        // } else if(tokens[0] == "discard" || tokens[0] == "di"){
        //     PrintDiscard();
        //     return true;
        // } else if(tokens[0] == "hand" || tokens[0] == "ha"){
        //     PrintHand();
        //     return true;
        // } else if(tokens[0] == "played" || tokens[0] == "pd"){
        //     PrintPlayArea();
        //     return true;
        } else if(tokens[0] == "deck" || tokens[0] == "d"){
            PrintAll();
            return true;
        } else if(tokens[0] == "autoclaim" || tokens[0] == "ac"){
            ToggleAutoClaim(tokens.size() > 1 ? tokens[1] : "");
            return true;
        } else if(tokens[0] == "help" || tokens[0] == "h"){
            PrintHelp();
            return true;
        } else if(tokens[0] == "rules" || tokens[0] == "r"){
            PrintRules();
            return true;
        }
        return false;
    }

    /*
    If autoclaim is on, ClaimAll() is called automatically upon entering the buy phase.
    */
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


    /*
    Handles player's action/buy phase. Returns true if buy phase should occur, false if it should be skipped.
    */
    virtual bool PlayPhase(bool isBuyPhase){
        while(GameShouldContinue(*shop)){
            PrintStatus(true);
            cout << "[" << name << " - " << (isBuyPhase ? "buy" : "action") << "]: ";
            vector<string> tokens = ResponseToTokens("");
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
                    if(isBuyPhase){
                        continue;
                    } else{
                        // move to buy phase
                        return true;
                    }
                }
            }
            cout << "Invalid input.\n";
        }
        return true;
    }

    /*
    Handles clenup / end phase
    */
    void EndTurn(){
        MoveAllCards(hand, discardPile);
        MoveAllCards(playArea, discardPile);
        Draw(5);
    }

    // ------------------------------------------------ PRINTING -------------------------------------------------------

    /*
    Functions used to display the state of the game.
    */

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
        PrintDeck();
    }
    void PrintDeck(){
        map<string, size_t> cards;
        vector<Card> deck = GetDeck();
        for(Card c : deck){
            cards[c.data.name]++;
        }
        cout << "Deck (" << deck.size() << "): ";
        for(auto const& [cardName, count] : cards){
            cout << count << "x " << cardName;
            cout << ", ";
        }
        cout << "\b\b " << "\n";
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
        cout << "\n";
        cout << "---------------- SHOP ----------------\n";
        cout << "CT  NAME   COST         DESCRIPTION\n";
        for(auto shopStack : *shop){
            if(shopStack.size() == 0){
                cout << "Empty\n";
            } else {
                CardData data = shopStack.back().data;
                if(lastType != CardType::ACTION && data.type != lastType){
                    cout << "\n";
                    lastType = data.type;
                }
                cout << shopStack.size() << "x " << data.name << " (" << data.cost << ")\t" << (data.name.length() <= 7 ? " \t" : "") << data.description << "\n";
            }
        }
        cout << "--------------------------------------\n";
        cout << "\n";
    }
    void PrintHelp(){
        cout << "\n";
        cout << "A CLI adaptation of Dominion (2008) by budjetti - version " + VERSION_NUMBER + "\n";
        cout << "https://github.com/budjetti/dominion\n";
        cout << "\n";
        cout << "Type 'help' or 'h' to see this list of commands.\n";
        // cout << "Type 'help name' or 'h name' to find more about name\n";
        cout << "When selecting a card for the buy/play commands or any other effects such as attack responses, the first card that unambiguously matches your provided input is selected. For example, if your hand is Cellar, Chapel and Laboratory, you only need to type 'play l' to play Laboratory or 'play ce' to play Cellar.\n";
        cout << "\n";
        cout << "COMMAND\t\t\t\tEXAMPLE USAGE\n";
        cout << "(a)uto(c)laim [off/on]\t\tautoclaim on / ac off / ac\n";
        cout << "(b)uy [NAME]\t\t\tbuy copper / b co\n";
        cout << "(c)laim\t\t\t\tclaim / c\n";
        cout << "(d)eck\t\t\t\tdeck / d\n";
        cout << "(e)nd\t\t\t\tend / e\n";
        cout << "(h)elp\t\t\t\thelp / h\n";
        cout << "(p)lay [NAME]\t\t\tplay smithy / p smi / p s\n";
        cout << "(r)ules\t\t\t\trules / r\n";
        cout << "(s)hop\t\t\t\tshop / s\n";
        cout << "\n";
    }
    void PrintRules(){
        cout << "---------------- RULES ---------------\n";
        cout << "\n";
        cout << "This is a quick summary of the rules, based on https://wiki.dominionstrategy.com/index.php/Gameplay\n";
        cout << "\n";
        cout << "In a game of Dominion, each player is given a starting deck of 10 cards, and they play around a Supply of card piles that they can buy from over the course of the game.\n\n";
        cout << "On their turn, a player goes through three turn phases:\n\n";
        cout << " - Action: They may play one Action card.\n";
        cout << " - Buy: They may play (claim) their Treasure cards and buy one card that they can afford, putting that card in their discard pile.\n";
        cout << " - Clean-up: They take all the cards they've played, and all cards remaining in their hand, and put them into their discard pile. They then draw 5 more cards, and end their turn.\n\n";
        cout << "When any player needs to draw cards and there are not enough cards left in their deck (draw pile) to do so, they reshuffle their discard pile to create a new deck. In this way cards that have been bought on earlier turns will be drawn on later turns to be played.\n\n";
        cout << "The game ends when either 3 Supply piles are empty, or when the Province pile empties. The player with the most victory points wins.\n\n";
        cout << "Many effects can allow the player to play more than one Action card or buy more than one card per turn. There are exceptions to almost nverything else just listed above as well.\n";
        cout << "\n";
        cout << "--------------------------------------\n";
    }

    // ------------------------------------------------ PLAY -------------------------------------------------------

    /*
    Functions used to resolve card effects. Returns false if played incorrectly.
    */

    // TREASURE

    bool PlayCopper(){
        gold++;
        return true;
    }
    bool PlaySilver(){
        gold += 2;
        return true;
    }
    bool PlayGold(){
        gold += 3;
        return true;
    }

    // ACTION

    bool PlaySmithy(){
        Draw(3);
        return true;
    }
    bool PlayVillage(){
        Draw(1);
        actions += 2;
        return true;
    }
    bool PlayFestival(){
        actions += 2;
        buys++;
        gold += 2;
        return true;
    }
    bool PlayLaboratory(){
        Draw(2);
        actions++;
        return true;
    }
    bool PlayCellar(){
        actions++;
        vector<string> tokens = ResponseToTokens("Discard (eg. estate estate copper / e e co / all): ");
        size_t discardedCount = 0;
        if(tokens[0] == "all"){
            // doing a for loop here seems to not discard the last card...
            while(hand.size() > 0){
                Discard(hand[0].data.id);
                discardedCount++;
            }
        } else {
            for(string name : tokens){
                Card cardToDiscard = FindCard(name, hand);
                for(Card & card : hand){
                    if(card == cardToDiscard){
                        Discard(card.data.id);
                        discardedCount++;
                    }
                }
            }
        }
        Draw(discardedCount);
        return true;
    }
    bool PlayChapel(){
        vector<string> tokens = ResponseToTokens("Trash (eg. estate estate copper / e e co / all): ");
        if(tokens.size() == 0){
            return false;
        }
        if(tokens[0] == "all"){
            // doing a for loop here seems to not trash the last card...
            while(hand.size() > 0){
                Trash(hand[0].data.id);
            }
            return true;
        }
        for(string t : tokens){
            Trash(FindCard(t, hand).data.id);
        }
        return true;
    }
    bool PlayWorkshop(){
        vector<string> tokens = ResponseToTokens("Gain a card costing up to 4 (eg. gardens, village): ");
        if(tokens.size() == 0){
            cout << "Please input a card\n";
            return false;
        }
        Card cardToGain = FindCard(tokens[0], *shop);
        if (cardToGain.data.cost > 4){
            cout << "Too expensive\n";
            return false;
        } else {
            if(GainCard(cardToGain.data.name)){
                cout << "Gained " << cardToGain.data.name << "\n";
                return true;
            } else {
                cout << "Could not gain " << cardToGain.data.name << "\n";
                return false;
            }
        }
        cout << "Could not find card " << tokens[0] << " in shop.\n";
        return false;
    }
    bool PlayLibrary(){
        vector<Card> aside;
        while(hand.size() < 7 && (drawPile.size() > 0 || discardPile.size() > 0)){
            if(drawPile.size() == 0){
                ShuffleDiscardIntoDraw();
            }
            // TODO check if this should be front or back
            Card drawTop = drawPile.back();
            if(drawTop.data.type == CardType::ACTION){
                cout << "Set aside " << drawTop.data.name << "? (y/n): ";
                if(Confirm()){
                    // move TOP card of draw pile aside
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
        return true;
    }
    bool PlayWoodcutter(){
        buys++;
        gold += 2;
        return true;
    }
    bool PlayMoneylender(){
        if(Trash(CardId::COPPER)){
            cout << "Trashed Copper for 3 gold\n";
            gold += 3;
        } else {
            cout << "Failed to trash Copper\n";
        }
        return true;
    }
    bool PlayMine(){
        if(hand.size() == 0 || FindCardsOfType(CardType::TREASURE, hand).size() == 0){
            cout << "No treasure cards in hand.\n";
            // returning false here causes softlock
            return true;
        }
        vector<string> tokens = ResponseToTokens("Trash treasure from hand (eg. copper / co): ");
        if(tokens.size() == 0){
            return false;
        }
        Card treasureToTrash = FindCard(tokens[0], hand);
        if(treasureToTrash.data.type != CardType::TREASURE){
            cout << "Not a treasure card.\n";
            return false;
        }
        size_t newCost = treasureToTrash.data.cost + 3;
        cout << "Gain treasure with cost " << newCost << " or less (eg. silver / si): ";
        vector<string> tokens2 = ResponseToTokens("");
        if(tokens2.size() == 0){
            return false;
        }
        Card treasureToGain = FindCard(tokens2[0], *shop);
        if(treasureToTrash.data.type != CardType::TREASURE){
            cout << "Not a treasure card.\n";
            return false;
        }
        if(treasureToGain.data.cost > newCost){
            cout << "Too expensive.\n";
            return false;
        }
        GainCard(treasureToGain.data.name, &hand);
        cout << "Gained " << treasureToGain.data.name << " into hand.\n";
        return true;
    }
    bool PlayFeast(){
        vector<string> tokens = ResponseToTokens("Gain card with cost 5 or less (eg. laboratory / lab): ");
        if(tokens.size() == 0){
            return false;
        }
        Card cardToGain = FindCard(tokens[0], *shop);
        if(cardToGain.data.cost <= 5){
            GainCard(cardToGain.data.name);
            cout << "Gained " << cardToGain.data.name << "\n";
        } else {
            cout << "Too expensive\n";
            return false;
        }
        // using MoveCard instead of Trash because the card is being moved from play area
        MoveCard(playArea, *trash, CardId::FEAST);
        return true;
    }
    bool PlayThroneRoom(){
        vector<string> tokens = ResponseToTokens("Choose card from hand to play twice (eg. witch / throne): ");
        if(tokens.size() == 0){
            return false;
        }
        Card card = FindCard(tokens[0], hand);
        if(card.data.type != CardType::ACTION){
            cout << "Not an action card\n";
            return false;
        }
        actions++;
        PlayCard(card.data.name, CardType::ACTION);
        ResolveEffect(card.data.id);
        return true;
    }
    bool PlayMarket(){
        Draw(1);
        actions++;
        buys++;
        gold++;
        return true;
    }
    bool PlayRemodel(){
        if(hand.size() == 0){
            cout << "No card to trash.\n";
            return false;
        }
        vector<string> tokens = ResponseToTokens("Trash card from hand (eg. silver / si): ");
        if(tokens.size() == 0){
            PlayRemodel();
            return false;
        }
        Card cardToTrash = FindCard(tokens[0], hand);
        if(cardToTrash.data.id == CardId::NO_ID){
            cout << "No card " << tokens[0] << " found in hand.\n";
            return false;
        }
        size_t newCardCost = cardToTrash.data.cost + 2;
        cout << "Gain card with cost " << newCardCost << " or less (eg. copper): ";
        vector<string> tokens2 = ResponseToTokens("");
        if(tokens.size() == 0){
            return false;
        }
        Card cardToGain = FindCard(tokens2[0], *shop);
        if(cardToGain.data.cost <= newCardCost){
            GainCard(cardToGain.data.name);
            cout << "Gained " << cardToGain.data.name << " \n";
        } else {
            cout << "Too expensive.\n";
            return false;
        }
        Trash(cardToTrash.data.id);
        return true;
    }
    bool PlayChancellor(){
        gold += 2;
        cout << "Move " << drawPile.size() << " cards from draw to discard? (y/n): ";
        if(autoResponse || Confirm()){
            MoveAllCards(drawPile, discardPile);
            cout << "Cards moved.\n";
        }
        return true;
    }
    bool PlayAdventurer(){
        vector<Card> aside;
        size_t found = 0;
        while(found < 2 && (drawPile.size() > 0 || discardPile.size() > 0)){
            if(drawPile.size() == 0){
                ShuffleDiscardIntoDraw();
            }
            Card drawTop = drawPile.back();
            if(drawTop.data.type != CardType::TREASURE){
                // if top card is not a treausre, "reveal it" (set it aside)
                vector<Card>::iterator pos = drawPile.end();
                Card copy(drawTop.data.id);
                aside.push_back(copy);
                drawPile.erase(pos);
                continue;
            }
            // if it was a treasure, draw
            Draw(1);
            found++;
        }
        MoveAllCards(aside, discardPile);
        return true;
    }
    bool PlayCouncilRoom(){
        Draw(4);
        buys++;
        for(Player * p : *allPlayers){
            if(p->name != name){
                p->Draw(1);
            }
        }
        return true;
    }

    // ACTION - ATTACK

    vector<Card> AttackResponse(CardId attackId){
        vector<Card> selected;

        cout << name << " is attacked by " << IdToCardData(attackId).name << "\n";
        if(FindCard("Moat", hand).data.id != CardId::NO_ID){
            if(!autoResponse)
                cout << "Cancel attack with Moat? (Y/n): ";
            if(autoResponse || Confirm("y"))
                return selected;
        }

        // bureaucrat
        vector<Card> victoryCards;

        // bureaucrat, militia
        vector<string> response;

        switch (attackId)
        {
        case CardId::BUREAUCRAT:
            victoryCards = FindCardsOfType(CardType::VICTORY, hand);
            if(victoryCards.size() == 0){
                cout << "No victory cards in hand\n";
                return selected;
            }
            if(victoryCards.size() == 1){
                MoveCard(hand, drawPile, victoryCards[0].data.id);
                cout << "Moved " << victoryCards[0].data.name << " to top of draw pile\n";
                return selected;
            }
            cout << "Victory cards in hand: ";
            PrintCardVector(victoryCards);
            while(1){
                if(autoResponse){
                    vector<string> victoryCards{"Province", "Duchy", "Gardens", "Estate"};
                    for(string s : victoryCards){
                        Card c = FindCard(s, hand).data.id;
                        if(c.data.id != CardId::NO_ID){
                            MoveCard(hand, drawPile, c.data.id);
                            return selected;
                        }
                    }
                    cout << "Something went wrong\n";
                    return selected;
                }
                response = ResponseToTokens("Choose card to put on top (eg. estate / es): ");
                Card card = FindCard(response[0], hand);
                if(card.data.type == CardType::VICTORY){
                    MoveCard(hand, drawPile, card.data.id);
                    return selected;
                }
            }
            break;

        case CardId::SPY:
            if(drawPile.size() == 0){
                ShuffleDiscardIntoDraw();
            }
            if(drawPile.size() > 1){
                Card copy(drawPile.back().data.id);
                selected.push_back(copy);
            }
            break;

        case CardId::THIEF:
            if(drawPile.size() < 2){
                ShuffleDiscardIntoDraw();
            }
            for(int i = 0; i < 2; i++){
                if(drawPile.size() <= i){
                    break;
                }
                Card copy(drawPile.rbegin()[i].data.id);
                selected.push_back(copy);
            }
            break;

        case CardId::MILITIA:
            while(hand.size() > 3){
                if(autoResponse){
                    vector<CardId> discardOrder{
                        // unplayable
                        CardId::PROVINCE,
                        CardId::DUCHY,
                        CardId::GARDENS,
                        CardId::ESTATE,
                        CardId::CURSE,
                        // 0
                        CardId::COPPER,
                        // 2
                        CardId::MOAT,
                        CardId::CELLAR,
                        CardId::CHAPEL,
                        // 3
                        CardId::SILVER,
                        CardId::CHANCELLOR,
                        CardId::WOODCUTTER,
                        CardId::VILLAGE,
                        // 4
                        CardId::BUREAUCRAT,
                        CardId::SMITHY,
                        CardId::MILITIA,
                        CardId::MONEYLENDER,
                        CardId::WORKSHOP,
                        CardId::THRONE_ROOM,
                        CardId::FEAST,
                        CardId::SPY,
                        CardId::THIEF,
                        CardId::REMODEL,
                        // 5
                        CardId::LABORATORY,
                        CardId::COUNCIL_ROOM,
                        CardId::FESTIVAL,
                        CardId::MARKET,
                        CardId::WITCH,
                        CardId::MINE,
                        CardId::LIBRARY,
                        // 6
                        CardId::GOLD,
                        CardId::ADVENTURER,
                    };
                    for(CardId id : discardOrder){
                        for(int i = 0; i < hand.size() - 3; i++){
                            if(hand.size() <= 3)
                                break;
                            string s = IdToCardData(id).name;
                            // cout << "looking for " << s << "\n";
                            CardId id = FindCard(s, hand).data.id;
                            if(id != CardId::NO_ID && Discard(id)){
                                cout << name << " discarded " << s << "\n";
                            }
                        }
                    }
                } else {
                    PrintHand();
                    cout << "Discard " << hand.size() - 3 << " cards (eg. est est pro): ";
                    response = ResponseToTokens("");
                    for(string s : response){
                        if(hand.size() <= 3){
                            // can't discard too many cards
                            break;
                        }
                        if(Discard(FindCard(s, hand).data.id)){
                            cout << name << " discarded " << s << "\n";
                        }
                    }
                }
            }
            break;

        case CardId::WITCH:
            GainCard("Curse");
            break;

        default:
            break;
        }
        return selected;
    }

    /*
    Handles the attack portion, which is blockable by moat, of attack cards.
    */
    bool Attack(CardId attackId){
        for(Player * p : *allPlayers){
            // Spy also targets self
            if(p->name != name || attackId == CardId::SPY){
                vector<Card> selected = p->AttackResponse(attackId);

                // Thief needs a response
                if(attackId == CardId::THIEF){
                    cout << p->name << " revealed: ";
                    PrintCardVector(selected);
                    for(Card c : selected){
                        if(c.data.type == CardType::TREASURE){
                            cout << "Steal " << c.data.name << "? (y/n): ";
                            if(Confirm()){
                                p->Trash(c.data.id, true);
                                GainCard(c.data.name);
                                cout << "Stole " << c.data.name << " from " << p->name << "\n";
                                break;
                            }
                        }
                    }
                } else if(attackId == CardId::SPY){
                    if(selected.size() == 0){
                        cout << p->name << " has no cards to reveal\n";
                    } else {
                        cout << "Make " << p->name << " discard " << selected[0].data.name << "? (y/n): ";
                        if(Confirm()){
                            p->MoveCard(p->drawPile, p->discardPile);
                        }
                    }
                }
            }
        }
        return true;
    }

    bool PlayBureaucrat(){
        GainCard("Silver", &drawPile);
        Attack(CardId::BUREAUCRAT);
        return true;
    }
    bool PlayThief(){
        Attack(CardId::THIEF);
        return true;
    }
    bool PlaySpy(){
        actions++;
        Draw(1);
        Attack(CardId::SPY);
        return true;
    }
    bool PlayWitch(){
        Draw(2);
        Attack(CardId::WITCH);
        return true;
    }
    bool PlayMilitia(){
        gold += 2;
        Attack(CardId::MILITIA);
        return true;
    }

    // ACTION - REACTION

    // Moat's secondary effect is implemented in Attack()
    bool PlayMoat(){
        Draw(2);
        return true;
    }
};

class Bot : public Player{
public:
    Bot(string name, vector<vector<Card>>* shop, vector<Card>* trash, vector<Player*>* allPlayers) :
        Player(name, shop, trash, allPlayers)
    {
        autoResponse = true;
    }

    void TakeTurn() override{
        cout << name << "'s turn\n";
        Player::TakeTurn();
    }

protected:
    vector<CardId> playOrder{
        // non-terminal
        CardId::LABORATORY,
        CardId::VILLAGE,
        CardId::MARKET,
        CardId::FESTIVAL,
        // terminal
        CardId::WITCH,
        CardId::COUNCIL_ROOM,
        CardId::SMITHY,
        CardId::BUREAUCRAT,
        CardId::MILITIA,
        CardId::CHANCELLOR,
        CardId::WOODCUTTER,
        CardId::MOAT,
        // requires logic
        /*
        CardId::CELLAR,
        CardId::MONEYLENDER,
        CardId::WORKSHOP,
        CardId::THRONE_ROOM,
        CardId::CHAPEL,
        CardId::FEAST,
        CardId::SPY,
        CardId::THIEF,
        CardId::REMODEL,
        CardId::MINE,
        CardId::LIBRARY,
        CardId::ADVENTURER,
        */
    };

    vector<CardId> buyOrder{
        // 8
        CardId::PROVINCE,
        // 6
        CardId::GOLD,
        // 5
        CardId::DUCHY,
        CardId::COUNCIL_ROOM,
        CardId::LABORATORY,
        CardId::FESTIVAL,
        CardId::MARKET,
        CardId::WITCH,
        // 4
        CardId::GARDENS,
        CardId::SMITHY,
        CardId::MILITIA,
        CardId::BUREAUCRAT,
        // 3
        CardId::SILVER,
        CardId::CHANCELLOR,
        CardId::WOODCUTTER,
        CardId::VILLAGE,
        // 2
        CardId::MOAT,
        // requires logic
        /*
        CardId::MONEYLENDER,
        CardId::WORKSHOP,
        CardId::THRONE_ROOM,
        CardId::CHAPEL,
        CardId::FEAST,
        CardId::SPY,
        CardId::THIEF,
        CardId::REMODEL,
        CardId::MINE,
        CardId::LIBRARY,
        CardId::ADVENTURER,
        CardId::CELLAR,
        */
    };

    size_t ProvinceCount(){
        for(vector<Card> v : *shop){
            if(v.size() == 0){
                continue;
            } else if(v.back().data.id == CardId::PROVINCE){
                return v.size();
            }
        }
        // cout << "No Provinces in shop\n";
        return 0;
    }

    bool PlayNextCard(){
        for(CardId id : playOrder){
            if(PlayCard(IdToCardData(id).name, CardType::ACTION, false))
                return true;
        }
        return false;
    }

    const bool IsLateGame(){
        return (ProvinceCount() <= 4 || EmptySupplyPileCount(*shop) >= 2);
    }

    const int RandInt(int min, int max){
        unsigned seed = chrono::system_clock::now().time_since_epoch().count();
        mt19937 generator(seed);
        uniform_int_distribution<int> distribution(min, max);
        return distribution(generator);
    }

    bool PlayPhase(bool isBuyPhase) override{
        // cout << "bot taking turn\n";
        while(GameShouldContinue(*shop)){
            // cout << "claiming\n";
            ClaimAll();

            if(!isBuyPhase){
                while(PlayNextCard());
                // move to buy phase
                return true;
            } else {
                for(int i = 0; i < buys; i++){
                    for(CardId id : buyOrder){
                        CardData data = IdToCardData(id);
                        if(data.id == CardId::PROVINCE){
                            // always buy province when able
                            if(BuyCard(data.name, false)){
                                break;
                            }
                        } else if(data.type == CardType::VICTORY && !IsLateGame()){
                            // Only buy non-province victory cards in late game
                            continue;
                        } else if(data.id == CardId::GOLD && IsLateGame()){
                            // Prefer Duchy over gold in late game
                            if(BuyCard("Duchy", false)){
                                break;
                            }
                        } else if(data.id == CardId::SILVER){
                            // buy silver 8/9 times
                            if(RandInt(1,3) != 1){
                                if(BuyCard("Silver", false)){
                                    break;
                                }
                            }
                        }

                        // 1/3 chance to skip card in buy order
                        if(RandInt(1, 3) == 1){
                            continue;
                        }

                        if(BuyCard(data.name, false)){
                            break;
                        }
                    }
                }
                return true;
            }
        }
        return true;
    }
};

/*
Used for sorting shop stacks.
*/
static const bool GreaterCost(CardId a, CardId b){
    Card first(a); Card second(b);
    return first.data.cost < second.data.cost;
}

/*
Starts, advances and ends game. Tells players when to take their turns. Handles common areas like shop and trash.
*/
class Game{
public:
    Game() : playerCount(0), botCount(0){}

    /*
    Spela dataspel.
    */
    void Start(){
        // TODO setup options

        // Set player/bot count
        Settings();

        // Set up game according to settings
        Setup();

        // Play until game end condition have been met
        while(PlayRound());

        // Results
        EndGame();
    }

    ~Game(){
        // delete players?
    }

private:

    // Member variables
    vector<Player*> players;
    vector<vector<Card>> shop;
    vector<Card> trash;
    int playerCount;
    int botCount;

    /*
    Creates shop stack. TODO add a dedicated shop stack class?
    */
    void AddShopStack(CardId id, int count){
        vector<Card> cards;
        for(size_t i = 0; i < count; i++){
            Card card(id);
            cards.push_back(card);
        }
        shop.push_back(cards);
    }

    void CreatePlayer(string name, bool isBot){
        if(isBot){
            Bot * bot = new Bot(name, &shop, &trash, &players);
            players.push_back(bot);
        } else {
            Player * player = new Player(name, &shop, &trash, &players);
            players.push_back(player);
        }
    }
    bool tryParse(string& input, int& output){
        try{
            output = stoi(input);
        } catch (invalid_argument){
            return false;
        }
        return true;
    }

    void GetCount(string prompt, int& count, int max){
        string input;
        cout << prompt << " (0-" << max << "): ";
        getline(cin, input);
        while(!tryParse(input, count)){
            cout << prompt << " (0-" << max << "): ";
            getline(cin, input);
        }
        if(count > max || count < 0){
            GetCount(prompt, count, max);
        }
    }
    void Settings(){
        cout << "A CLI adaptation of Dominion (2008) by budjetti - version " + VERSION_NUMBER + "\n";
        // cout << "https://github.com/budjetti/dominion\n";
        cout << "\n";
        string input;

        GetCount("Enter player count", playerCount, 4);
        int maxBots = 4 - playerCount;
        GetCount("Enter bot count", botCount, maxBots);
    }

    /*
    Create players and shop.
    */
    void Setup(){
        vector<string> playerNames{"Player One", "Player Two", "Player Three", "Player Four"};
        for(int i = 0; i < playerCount; i++){
            CreatePlayer(playerNames[i], false);
        }
        vector<string> botNames{"Bot One", "Bot Two", "Bot Three", "Bot Four"};
        for(int i = 0; i < botCount; i++){
            CreatePlayer(botNames[i], true);
        }

        // create shop
        size_t victoryCount = players.size() > 2 ? 12 : 8;
        if(debug){
            victoryCount = 1;
        }
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

        // 10 RANDOMLY SELECTED CARDS
        vector<CardId> selected{
            // 2
            CardId::CELLAR,
            CardId::CHAPEL,
            CardId::MOAT,
            // 3
            CardId::WORKSHOP,
            CardId::VILLAGE,
            CardId::WOODCUTTER,
            CardId::CHANCELLOR,
            // 4
            CardId::MONEYLENDER,
            CardId::GARDENS,
            CardId::SMITHY,
            CardId::THRONE_ROOM,
            CardId::BUREAUCRAT,
            CardId::FEAST,
            CardId::SPY,
            CardId::MILITIA,
            CardId::THIEF,
            CardId::REMODEL,
            // 5
            CardId::WITCH,
            CardId::COUNCIL_ROOM,
            CardId::MARKET,
            CardId::MINE,
            CardId::FESTIVAL,
            CardId::LIBRARY,
            CardId::LABORATORY,
            // 6
            CardId::ADVENTURER,
        };
        unsigned seed = chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(selected.begin(), selected.end(), default_random_engine(seed));
        selected.erase(selected.begin() + 10, selected.end());
        sort(selected.begin(), selected.end(), GreaterCost);
        for(CardId id : selected){
            AddShopStack(id, (id == CardId::GARDENS ? victoryCount : 10));
        }
    }

    /*
    Lets each player take a turn. Returns false if game should end.
    */
    bool PlayRound(){
        if(players.size() == 0)
            return false;

        for(auto * p : players){
            cout << "\n---------- " << p->GetName() <<  "'s Turn ---------\n\n";
            p->TakeTurn();
            if(!GameShouldContinue(shop)){
                return false;
            }
        }
        return true;
    }

    /*
    Print results.
    */
    void EndGame(){
        cout << "\n---------- GAME FINISHED --------\n\n";
        for(Player * p : players){
            p->PrintScore();
        }
    }
};

/*
Segmentation fault jumpscare
*/
int main(){
    Game g;
    g.Start();
}
