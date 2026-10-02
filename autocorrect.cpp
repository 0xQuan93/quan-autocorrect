#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_map>
#include <vector>
#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputcontextmanager.h>
#include <fcitx/inputcontextproperty.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/inputpanel.h>
#include <fcitx/instance.h>
#include <fcitx/userinterface.h>
#include <spell_public.h>

using namespace fcitx;

namespace quan {
// Explicit, conservative replacements. Unknown words are never guessed.
const std::unordered_map<std::string, std::string> corrections = {
    {"teh", "the"}, {"hte", "the"}, {"thier", "their"},
    {"recieve", "receive"}, {"recieved", "received"}, {"recieving", "receiving"},
    {"definately", "definitely"}, {"defintely", "definitely"},
    {"seperate", "separate"}, {"seperately", "separately"},
    {"occured", "occurred"}, {"occuring", "occurring"},
    {"accomodate", "accommodate"}, {"accomodation", "accommodation"},
    {"acheive", "achieve"}, {"acheived", "achieved"},
    {"beleive", "believe"}, {"beleiveing", "believing"},
    {"becuase", "because"}, {"becasue", "because"}, {"beacuse", "because"},
    {"alot", "a lot"}, {"agian", "again"}, {"agaisnt", "against"},
    {"adn", "and"}, {"nad", "and"}, {"abotu", "about"}, {"baout", "about"},
    {"aboutt", "about"}, {"adress", "address"}, {"addres", "address"},
    {"arguement", "argument"}, {"asistant", "assistant"},
    {"avaliable", "available"}, {"begining", "beginning"},
    {"buisness", "business"}, {"busines", "business"},
    {"calender", "calendar"}, {"comming", "coming"}, {"concious", "conscious"},
    {"comand", "command"}, {"commited", "committed"}, {"commiting", "committing"},
    {"compatable", "compatible"}, {"compatability", "compatibility"},
    {"consistant", "consistent"}, {"contex", "context"},
    {"dependancy", "dependency"}, {"developement", "development"},
    {"desparate", "desperate"}, {"dissapear", "disappear"},
    {"dissapoint", "disappoint"}, {"embarass", "embarrass"},
    {"enviroment", "environment"}, {"enviornment", "environment"},
    {"existance", "existence"}, {"experiance", "experience"},
    {"explaination", "explanation"}, {"fucntion", "function"},
    {"funtion", "function"}, {"garantee", "guarantee"},
    {"finaly", "finally"}, {"foriegn", "foreign"}, {"freind", "friend"},
    {"freinds", "friends"}, {"foward", "forward"}, {"goign", "going"},
    {"heigth", "height"}, {"isntall", "install"}, {"intalled", "installed"},
    {"jsut", "just"}, {"liek", "like"}, {"maintainance", "maintenance"},
    {"mkae", "make"}, {"makign", "making"}, {"memroy", "memory"},
    {"mesage", "message"},
    {"happend", "happened"}, {"happenned", "happened"},
    {"havent", "haven't"}, {"hasnt", "hasn't"}, {"hadnt", "hadn't"},
    {"dont", "don't"}, {"doesnt", "doesn't"}, {"didnt", "didn't"},
    {"isnt", "isn't"}, {"wasnt", "wasn't"}, {"werent", "weren't"},
    {"couldnt", "couldn't"}, {"wouldnt", "wouldn't"}, {"shouldnt", "shouldn't"},
    {"youre", "you're"}, {"theyre", "they're"}, {"youve", "you've"},
    {"weve", "we've"}, {"theyve", "they've"},
    {"immediatly", "immediately"}, {"independant", "independent"},
    {"intresting", "interesting"}, {"langauge", "language"},
    {"neccessary", "necessary"}, {"neccesary", "necessary"},
    {"necesary", "necessary"}, {"noticable", "noticeable"},
    {"ocassion", "occasion"}, {"ocassionally", "occasionally"},
    {"occurance", "occurrence"}, {"occurence", "occurrence"},
    {"peice", "piece"}, {"peices", "pieces"}, {"peopel", "people"},
    {"poeple", "people"}, {"posession", "possession"},
    {"prefered", "preferred"}, {"privelege", "privilege"},
    {"probaly", "probably"}, {"probbaly", "probably"}, {"problme", "problem"},
    {"porject", "project"}, {"publically", "publicly"},
    {"realy", "really"}, {"realyl", "really"}, {"relevent", "relevant"},
    {"refered", "referred"}, {"remeber", "remember"},
    {"requset", "request"}, {"responce", "response"}, {"resposne", "response"},
    {"retrun", "return"}, {"runing", "running"}, {"rythm", "rhythm"},
    {"responsability", "responsibility"}, {"rigth", "right"},
    {"sentance", "sentence"}, {"shoudl", "should"}, {"similiar", "similar"},
    {"somthing", "something"}, {"succesful", "successful"},
    {"sucess", "success"}, {"sucessful", "successful"},
    {"thansk", "thanks"}, {"thnaks", "thanks"}, {"taht", "that"},
    {"tahn", "than"}, {"thats", "that's"}, {"theres", "there's"},
    {"tihs", "this"}, {"thsi", "this"}, {"tommorow", "tomorrow"},
    {"tomorow", "tomorrow"}, {"togehter", "together"},
    {"treshold", "threshold"}, {"truely", "truly"}, {"untill", "until"},
    {"unfortunatly", "unfortunately"}, {"udpate", "update"}, {"udpated", "updated"},
    {"usefull", "useful"}, {"usualy", "usually"}, {"wierd", "weird"},
    {"waht", "what"}, {"watn", "want"}, {"wether", "whether"},
    {"whcih", "which"}, {"wich", "which"}, {"wnat", "want"},
    {"woudl", "would"}, {"wiht", "with"}, {"owrk", "work"},
    {"witht", "with"}, {"writting", "writing"}, {"yuo", "you"},
    {"yuor", "your"}
};

std::string corrected(const std::string &word) {
    auto lower = word;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    auto it = corrections.find(lower);
    if (it == corrections.end()) return word;
    // Preserve mixed-case identifiers, names, and abbreviations.
    auto title = lower;
    if (!title.empty()) title[0] = std::toupper(static_cast<unsigned char>(title[0]));
    const bool upper = std::all_of(word.begin(), word.end(),
        [](unsigned char c) { return !std::isalpha(c) || std::isupper(c); });
    if (word != lower && word != title && !upper) return word;
    auto result = it->second;
    if (upper) std::transform(result.begin(), result.end(), result.begin(),
                            [](unsigned char c) { return std::toupper(c); });
    else if (word == title) result[0] = std::toupper(static_cast<unsigned char>(result[0]));
    return result;
}

struct State : InputContextProperty {
    std::string word;
    bool bypassToken = false;
    bool committedSeparator = false;
    bool suggestions = false;
    std::vector<std::string> choices;
};

class Choice : public CandidateWord {
public:
    Choice(std::string value, std::function<void(InputContext *, std::string)> pick)
        : CandidateWord(Text(value)), value_(std::move(value)), pick_(std::move(pick)) {}
    void select(InputContext *ic) const override { pick_(ic, value_); }
private:
    std::string value_;
    std::function<void(InputContext *, std::string)> pick_;
};

class Engine : public InputMethodEngine {
public:
    explicit Engine(Instance *instance) : instance_(instance) {
        instance_->inputContextManager().registerProperty("quan-autocorrect", &factory_);
    }
    bool eligible(InputContext *ic) const {
        auto flags = ic->capabilityFlags();
        const auto excluded = CapabilityFlags(CapabilityFlag::PasswordOrSensitive) | CapabilityFlag::Terminal |
            CapabilityFlag::NoSpellCheck | CapabilityFlag::Url | CapabilityFlag::Email |
            CapabilityFlag::Digit | CapabilityFlag::Number | CapabilityFlag::Disable;
        if ((flags & excluded) || !(flags & CapabilityFlag::Preedit)) return false;
        auto app = ic->program();
        std::transform(app.begin(), app.end(), app.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        for (auto name : {"alacritty", "kitty", "foot", "ghostty", "konsole", "terminal",
                          "wezterm", "code", "neovim", "nvim", "emacs", "jetbrains"})
            if (app.find(name) != std::string::npos) return false;
        return true;
    }
    void clear(InputContext *ic) {
        auto *s = ic->propertyFor(&factory_);
        s->word.clear(); s->choices.clear(); s->suggestions = false;
        s->committedSeparator = false;
        ic->inputPanel().reset();
        ic->updatePreedit();
        ic->updateUserInterface(UserInterfaceComponent::InputPanel);
    }
    void commit(InputContext *ic, bool fix, const std::string &suffix = {}) {
        auto *s = ic->propertyFor(&factory_);
        auto text = (fix ? corrected(s->word) : s->word) + suffix;
        clear(ic);
        if (!text.empty()) ic->commitString(text);
    }
    void update(InputContext *ic) {
        auto *s = ic->propertyFor(&factory_);
        ic->inputPanel().reset();
        Text preedit(s->word, TextFormatFlag::Underline);
        preedit.setCursor(static_cast<int>(s->word.size()));
        ic->inputPanel().setClientPreedit(preedit);
        auto replacement = corrected(s->word);
        if (replacement != s->word)
            ic->inputPanel().setAuxUp(Text("Space → " + replacement + "   Esc: keep spelling"));
        if (s->suggestions) {
            s->choices.clear();
            if (replacement != s->word) s->choices.push_back(replacement);
            if (auto *spell = instance_->addonManager().addon("spell", true)) {
                for (auto &word : spell->call<ISpell::hintWithProvider>("en_US", SpellProvider::Enchant, s->word, 5)) {
                    if (word != s->word && std::find(s->choices.begin(), s->choices.end(), word) == s->choices.end())
                        s->choices.push_back(word);
                    if (s->choices.size() == 5) break;
                }
            }
            auto list = std::make_unique<CommonCandidateList>();
            list->setPageSize(5);
            list->setSelectionKey({Key("Alt+1"), Key("Alt+2"), Key("Alt+3"), Key("Alt+4"), Key("Alt+5")});
            for (auto &word : s->choices) {
                list->append<Choice>(word, [this](InputContext *ctx, std::string value) {
                    clear(ctx); ctx->commitString(value);
                });
            }
            ic->inputPanel().setCandidateList(std::move(list));
            ic->inputPanel().setAuxDown(Text(s->choices.empty() ? "No suggestions" : "Alt+1…5: choose   Esc: keep spelling"));
        }
        ic->updatePreedit();
        ic->updateUserInterface(UserInterfaceComponent::InputPanel);
    }
    void keyEvent(const InputMethodEntry &, KeyEvent &event) override {
        if (event.isRelease() || event.key().isModifier()) return;
        auto *ic = event.inputContext();
        auto *s = ic->propertyFor(&factory_);
        // Keep Omarchy's CapsLock/XCompose sequences working in both modes.
        auto rawSym = event.rawKey().sym();
        if ((rawSym == FcitxKey_Multi_key ||
             (rawSym >= FcitxKey_dead_grave && rawSym <= FcitxKey_dead_greek) ||
             instance_->isComposing(ic)) &&
            !(event.key().states() & (KeyStates(KeyState::Ctrl) | KeyState::Alt | KeyState::Super))) {
            auto composed = instance_->processComposeString(ic, rawSym);
            commit(ic, false);
            if (composed && !composed->empty()) ic->commitString(*composed);
            s->bypassToken = true;
            event.filterAndAccept();
            return;
        }
        if (!eligible(ic)) { clear(ic); s->bypassToken = false; return; }
        auto key = event.key();
        auto sym = key.sym();
        if (!s->word.empty() && key.check(Key("Control+Alt+s").normalize())) {
            s->suggestions = !s->suggestions; update(ic); event.filterAndAccept(); return;
        }
        if (s->suggestions && key.states() == KeyState::Alt && sym >= FcitxKey_1 && sym <= FcitxKey_5) {
            auto index = static_cast<size_t>(sym - FcitxKey_1);
            if (index < s->choices.size()) {
                auto value = s->choices[index]; clear(ic); ic->commitString(value);
                event.filterAndAccept(); return;
            }
        }
        if (key.states() & (KeyStates(KeyState::Ctrl) | KeyState::Alt | KeyState::Super | KeyState::Hyper)) {
            commit(ic, false); s->bypassToken = true; return;
        }
        if (sym == FcitxKey_Escape && !s->word.empty()) {
            commit(ic, false); s->bypassToken = true; event.filterAndAccept(); return;
        }
        if (sym == FcitxKey_BackSpace && !s->word.empty()) {
            s->word.pop_back(); update(ic); event.filterAndAccept(); return;
        }
        if (sym == FcitxKey_space) {
            if (!s->word.empty()) { commit(ic, true, " "); event.filterAndAccept(); }
            else if (s->committedSeparator) {
                s->committedSeparator = false;
                ic->commitString(" ");
                event.filterAndAccept();
            }
            s->bypassToken = false; return;
        }
        // A few unambiguous separators should behave like Space. A period and
        // colon remain literal because they may be part of a domain, filename,
        // URL, version, or key/value token (for example, teh.com).
        auto c = Key::keySymToUnicode(sym);
        if (!s->word.empty() && (c == ',' || c == '!' || c == '?' || c == ';')) {
            commit(ic, true, std::string(1, static_cast<char>(c)));
            s->bypassToken = true;
            s->committedSeparator = true;
            event.filterAndAccept();
            return;
        }
        // Non-space keys commit literally: URLs, filenames, commands, Enter,
        // punctuation and focus navigation must never trigger guessed edits.
        bool letter = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        if (!s->bypassToken && (letter || (c == '\'' && !s->word.empty()))) {
            if (s->word.size() >= 64) { commit(ic, false); s->bypassToken = true; return; }
            s->word.push_back(static_cast<char>(c)); update(ic); event.filterAndAccept(); return;
        }
        commit(ic, false);
        s->bypassToken = !(sym == FcitxKey_Return || sym == FcitxKey_Tab);
    }
    void reset(const InputMethodEntry &, InputContextEvent &event) override {
        instance_->resetCompose(event.inputContext());
        clear(event.inputContext());
        event.inputContext()->propertyFor(&factory_)->bypassToken = false;
    }
    void deactivate(const InputMethodEntry &, InputContextEvent &event) override {
        auto *ic = event.inputContext();
        // Clients advertising this capability commit preedit themselves.
        if (event.type() == EventType::InputContextFocusOut &&
            (ic->capabilityFlags() & CapabilityFlag::ClientUnfocusCommit)) clear(ic);
        else commit(ic, false);
        instance_->resetCompose(ic);
        ic->propertyFor(&factory_)->bypassToken = false;
    }
private:
    Instance *instance_;
    SimpleInputContextPropertyFactory<State> factory_;
};

class Factory : public AddonFactory {
public:
    AddonInstance *create(AddonManager *manager) override { return new Engine(manager->instance()); }
};
}
FCITX_ADDON_FACTORY(quan::Factory)
