#include "displayapp/screens/Calculator.h"
#include "displayapp/DisplayApp.h"
#include <iostream>
#include <regex>
#include <string>
#include <cmath>
#include <sstream>
#include <array>

using namespace Pinetime::Applications::Screens;

static lv_style_t style_btn;
static lv_style_t style_equals_btn;
static lv_obj_t* label;

static void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
  auto* screen = static_cast<Calculator*>(obj->user_data);
  screen->OnButtonEvent(obj, event);
}

void removeTrailingZeros(std::string& str) {
  size_t pos = str.find_last_not_of('0');
  if (pos != std::string::npos && str[pos + 1] == '0' && str.find('.') != std::string::npos)
    str.erase(pos + 1);
  if (str[pos] == '.')
    str.erase(pos);
}

std::array<std::string, 2> getParts(const std::string& str, char op) {
  std::array<std::string, 2> parts;
  std::size_t pos = str.find(op);
  bool foundBracket = false;
  for (int i = static_cast<int>(pos) - 1; i >= 0; i--) {
    if (str[i] == '^' || str[i] == 'R' || str[i] == '*' || str[i] == '/' || str[i] == '+' || (str[i] == '-' && !foundBracket && op == '^'))
      break;
    if (str[i] == ']')
      foundBracket = true;
    parts[0] = str[i] + parts[0];
    if (str[i] == '[' && foundBracket)
      break;
    if (str[i] == '-' && !foundBracket)
      break;
  }
  foundBracket = false;
  for (int i = static_cast<int>(pos) + 1; i < static_cast<int>(str.length()); i++) {
    if (str[i] == '^' || str[i] == 'R' || str[i] == '*' || str[i] == '/' || str[i] == '+' ||
        (str[i] == '-' && !foundBracket && i != static_cast<int>(pos) + 1))
      break;
    if (str[i] == '[')
      foundBracket = true;
    parts[1] += str[i];
    if (str[i] == ']' && foundBracket)
      break;
  }
  return parts;
}

void calculate(std::string& str) {
  std::regex pattern("\\([^()]+\\)");
  std::smatch matches;

  while (std::regex_search(str, matches, pattern)) {
    for (auto& match : matches) {
      std::string match_str = match.str().substr(1, match.str().length() - 2);
      calculate(match_str);
      if (match_str[0] == '-')
        match_str = "[" + match_str + "]";
      str.replace(match.first, match.second, match_str);
    }
  }

  for (int i = 0; i < 3; i++) {
    char op1 = '+';
    char op2 = '-';
    if (i == 0) {
      op1 = '^';
      op2 = 'R';
    } else if (i == 1) {
      op1 = '*';
      op2 = '/';
    }
    size_t pos1 = str.find(op1);
    size_t pos2 = str.find(op2);
    while (pos1 != std::string::npos || pos2 != std::string::npos) {
      char op = pos1 < pos2 ? op1 : op2;
      size_t pos = op == op1 ? pos1 : pos2;
      std::array<std::string, 2> parts = getParts(str, op);
      int partLength = parts[0].length();
      int partLength2 = parts[1].length();
      if (parts[0].front() == '[' && parts[0].back() == ']') {
        parts[0] = parts[0].substr(1);
        parts[0] = parts[0].substr(0, parts[0].length() - 1);
      }
      if (parts[1].front() == '[' && parts[1].back() == ']') {
        parts[1] = parts[1].substr(1);
        parts[1] = parts[1].substr(0, parts[1].length() - 1);
      }
      if (op == 'R') {
        if (parts[0].empty())
          parts[0] = "2";
      }
      std::string result;
      if (op == '^') {
        result = std::to_string(std::pow(std::stof(parts[0]), std::stof(parts[1])));
      } else if (op == 'R') {
        result = std::to_string(std::pow(std::stof(parts[1]), 1 / std::stof(parts[0])));
      } else if (op == '*') {
        result = std::to_string(std::stof(parts[0]) * std::stof(parts[1]));
      } else if (op == '/') {
        result = std::to_string(std::stof(parts[0]) / std::stof(parts[1]));
      } else if (op == '+') {
        result = std::to_string(std::stof(parts[0]) + std::stof(parts[1]));
      } else {
        if (parts[0].length() == 0)
          result = "%" + parts[1];
        else
          result = std::to_string(std::stof(parts[0]) - std::stof(parts[1]));
      }
      removeTrailingZeros(result);
      if (std::isdigit(str[pos - partLength - 1]))
        result = "+" + result;
      str.erase(pos - partLength, partLength + 1 + partLength2);
      str.insert(pos - partLength, result);
      pos1 = str.find(op1);
      pos2 = str.find(op2);
    }

    size_t pos = 0;
    while ((pos = str.find("%", pos)) != std::string::npos) {
      str.replace(pos, 1, "-");
      pos += 1;
    }
  }
}

void makeButtons() {
  lv_style_init(&style_btn);
  lv_style_set_border_color(&style_btn, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_style_set_border_width(&style_btn, LV_STATE_DEFAULT, 3);
  lv_style_set_border_opa(&style_btn, LV_STATE_DEFAULT, LV_OPA_50);

  std::vector<std::string> labels = {"1", "2", "3", "(", ")", "4", "5", "6", "^", "R", "7", "8", "9", "*", "/", "C", "0", ".", "+", "-"};

  int x = 1;
  int y = 48;
  for (int i = 1; i <= 20; i++) {
    lv_obj_t* btn = lv_btn_create(lv_scr_act(), nullptr);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, 46, 46);
    lv_obj_add_style(btn, LV_BTN_PART_MAIN, &style_btn);
    lv_obj_set_event_cb(btn, btnEventHandler);

    lv_obj_t* label = lv_label_create(btn, nullptr);
    lv_label_set_text(label, labels[i - 1].c_str());
    lv_obj_align(label, nullptr, LV_ALIGN_CENTER, 0, 0);
    if (i % 5 == 0) {
      x = 1;
      y += 48;
    } else {
      x += 48;
    }
  }

  lv_style_init(&style_equals_btn);
  lv_style_set_radius(&style_equals_btn, LV_STATE_DEFAULT, 50);
  lv_style_set_border_color(&style_equals_btn, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_style_set_border_width(&style_equals_btn, LV_STATE_DEFAULT, 3);
  lv_style_set_border_opa(&style_equals_btn, LV_STATE_DEFAULT, LV_OPA_50);

  lv_obj_t* btn = lv_btn_create(lv_scr_act(), nullptr);
  lv_obj_set_pos(btn, 240 - 47, 1);
  lv_obj_set_size(btn, 46, 46);
  lv_obj_add_style(btn, LV_BTN_PART_MAIN, &style_equals_btn);
  lv_obj_set_event_cb(btn, btnEventHandler);

  lv_obj_t* label = lv_label_create(btn, nullptr);
  lv_label_set_text(label, "=");
  lv_obj_align(label, nullptr, LV_ALIGN_CENTER, 0, 0);
}

Calculator::Calculator() {
  makeButtons();

  label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(label, "");
  lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_long_mode(label, LV_LABEL_LONG_CROP);
  lv_label_set_align(label, LV_LABEL_ALIGN_RIGHT);
  lv_obj_set_pos(label, 0, 6);
  lv_obj_set_size(label, 240 - 48, 42);
}

Calculator::~Calculator() {
  lv_obj_clean(lv_scr_act());
}

void Calculator::OnButtonEvent(lv_obj_t* btn, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    lv_obj_t* btnLabel = lv_obj_get_child(btn, nullptr);
    const char* labelText = lv_label_get_text(btnLabel);
    if (labelText[0] == 'C') {
      char* labelText = lv_label_get_text(label);
      labelText[strlen(labelText) - 1] = '\0';
      lv_label_set_text(label, labelText);
    } else if (labelText[0] == '=') {
      std::string labelText(lv_label_get_text(label));
      calculate(labelText);
      lv_label_set_text(label, labelText.c_str());
    } else {
      std::string currentText(lv_label_get_text(label));
      currentText += labelText;
      lv_label_set_text(label, currentText.c_str());
    }
  }
}

bool Calculator::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if (event == Pinetime::Applications::TouchEvents::LongTap) {
    lv_label_set_text(label, "");
    return true;
  }
  return false;
}