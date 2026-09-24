enum class Role { System, User, Assistant };

class Message {
public:
    // Default-constructs an empty System message with empty content.
    // Needed so Conversation can allocate raw array slots before
    // append() fills them in.
    Message(){
        role_ = Role::system;
        content_ = "";
    }

    Message(Role role, std::string content){
        content_ = content;
        role_ = role;
    }

    Role               role()    const noexcept{
        return role;
    };  // Who sent this message.
    const std::string& content() const noexcept{
        return content_;
    };  // The message text.

private:
    Role        role_;
    std::string content_;
};