#include "autocorrect.cpp"
#include <fcitx-utils/eventdispatcher.h>
#include <iostream>
#include <stdexcept>

class Client : public InputContext {
public:
    std::string text;
    Client(InputContextManager &manager, const std::string &app = "typing-test")
        : InputContext(manager, app) { created(); setCapabilityFlags(CapabilityFlag::Preedit); }
    ~Client() override { destroy(); }
    const char *frontend() const override { return "test"; }
    void commitStringImpl(const std::string &s) override { text += s; }
    void deleteSurroundingTextImpl(int, unsigned int) override {}
    void forwardKeyImpl(const ForwardKeyEvent &) override {}
    void updatePreeditImpl() override {}
};
void require(bool value, const std::string &message) {
    if (!value) throw std::runtime_error(message);
}
void key(quan::Engine &engine, Client &client, const Key &value) {
    KeyEvent e(&client, value);
    engine.keyEvent(InputMethodEntry("english-autocorrect", "English Autocorrect", "en_US", "quan-autocorrect"), e);
    if (!e.accepted()) {
        auto c = Key::keySymToUnicode(value.sym());
        if (!(value.states() & (KeyStates(KeyState::Ctrl) | KeyState::Alt | KeyState::Super)) && c >= 32 && c < 127)
            client.text.push_back(static_cast<char>(c));
    }
}
void type(quan::Engine &engine, Client &client, const std::string &s) {
    for (unsigned char c : s) key(engine, client, Key(static_cast<KeySym>(c)));
}
int main() {
    char a0[] = "test-autocorrect", a1[] = "--disable=all", a2[] = "--enable=spell";
    char *argv[] = {a0, a1, a2};
    Instance instance(3, argv);
    instance.addonManager().registerDefaultLoader(nullptr);
    int result = 0;
    instance.eventDispatcher().schedule([&]() {
        try {
            quan::Engine engine(&instance);
            for (auto pair : std::vector<std::pair<std::string, std::string>>{
                    {"teh ", "the "}, {"Teh ", "The "}, {"TEH ", "THE "},
                    {"tEh ", "tEh "}, {"dont ", "don't "}, {"alot ", "a lot "},
                    {"jsut udpated teh porject ", "just updated the project "},
                    {"fucntion retrun requset resposne ", "function return request response "},
                    {"teh, teh! teh? teh; ", "the, the! the? the; "},
                    {"Teh! TEH? tEh, ", "The! THE? tEh, "},
                    {"Quan Zephyr Solana WaveWarZ ", "Quan Zephyr Solana WaveWarZ "},
                    {"teh.com teh@example.com teh_file teh123 /teh/path ", "teh.com teh@example.com teh_file teh123 /teh/path "},
                    {"hello, teh world ", "hello, the world "},
                    {"teh. ", "teh. "}, {std::string(80, 'a') + " ", std::string(80, 'a') + " "}}) {
                Client client(instance.inputContextManager());
                type(engine, client, pair.first);
                require(client.text == pair.second, "typing failed: " + pair.first + " => " + client.text);
            }
            for (auto flag : {CapabilityFlag::Password, CapabilityFlag::Sensitive,
                              CapabilityFlag::Terminal, CapabilityFlag::NoSpellCheck,
                              CapabilityFlag::Url, CapabilityFlag::Email, CapabilityFlag::Disable}) {
                Client client(instance.inputContextManager());
                client.setCapabilityFlags(CapabilityFlags(CapabilityFlag::Preedit) | flag);
                type(engine, client, "teh ");
                require(client.text == "teh ", "excluded field was corrected");
            }
            for (auto app : {"Alacritty", "com.mitchellh.ghostty", "code", "kitty", "foot", "org.kde.konsole"}) {
                Client client(instance.inputContextManager(), app);
                type(engine, client, "teh ");
                require(client.text == "teh ", "excluded application was corrected");
            }
            {
                Client client(instance.inputContextManager());
                type(engine, client, "teh"); key(engine, client, Key("Escape")); type(engine, client, " ");
                require(client.text == "teh ", "Escape did not keep original");
            }
            {
                Client client(instance.inputContextManager());
                type(engine, client, "teh! next ");
                require(client.text == "the! next ", "Corrected punctuation lost or duplicated its following space");
            }
            {
                Client client(instance.inputContextManager());
                type(engine, client, "tex"); key(engine, client, Key("BackSpace")); type(engine, client, "h ");
                require(client.text == "the ", "Backspace editing failed");
            }
            {
                Client client(instance.inputContextManager());
                type(engine, client, "teh"); key(engine, client, Key("Control+c"));
                require(client.text == "teh", "Shortcut did not flush literal preedit");
            }
            {
                Client a(instance.inputContextManager()), b(instance.inputContextManager());
                type(engine, a, "teh"); type(engine, b, "freind "); type(engine, a, " ");
                require(a.text == "the " && b.text == "friend ", "Client state leaked");
            }
            {
                Client client(instance.inputContextManager());
                type(engine, client, "teh"); key(engine, client, Key("Control+Alt+s"));
                require(client.inputPanel().candidateList() && client.inputPanel().candidateList()->size() > 0, "No spelling candidates");
                key(engine, client, Key("Alt+1"));
                require(client.text == "the", "Candidate selection failed");
            }
            {
                auto *spell = instance.addonManager().addon("spell", true);
                require(spell != nullptr, "Spell addon unavailable");
                auto words = spell->call<ISpell::hintWithProvider>("en_US", SpellProvider::Enchant, "helo", 5);
                require(!words.empty(), "English spelling dictionary unavailable");
                std::cout << "English dictionary suggestions for helo:";
                for (auto &word : words) std::cout << " " << word;
                std::cout << "\n";
            }
            {
                Client client(instance.inputContextManager());
                key(engine, client, Key("Multi_key"));
                key(engine, client, Key("apostrophe"));
                key(engine, client, Key("e"));
                require(client.text == "é", "Compose sequence failed: " + client.text);
            }
            std::cout << "PASS: corrections, case, literal tokens, excluded fields/apps, Escape, Backspace, shortcuts, context isolation, native spelling suggestions\n";
        } catch (const std::exception &e) { std::cerr << e.what() << '\n'; result = 1; }
        instance.exit();
    });
    instance.exec();
    return result;
}
