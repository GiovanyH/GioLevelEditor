/* Console */
std::vector<char*> console_items;
static int console_item_current = 0;

void console_window() {
    ImGui::SetNextWindowPos(ImVec2(10, 420));
    ImGui::SetNextWindowSize(ImVec2(1260, 290));
    ImGui::Begin("Console", 0);
    ImGui::ListBox(" ", &console_item_current, console_items.data(), console_items.size(), 10);
    ImGui::End();
}

void push_to_console(std::string string) {
    std::string* str = new std::string(string);
    console_items.push_back((char*)str->c_str());
    console_item_current++;
}

/* == */