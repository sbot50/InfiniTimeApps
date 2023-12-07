#pragma once

#include "displayapp/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Calculator : public Screen {
      public:
        Calculator();
        ~Calculator() override;
        void OnButtonEvent(lv_obj_t* obj, lv_event_t event);
        bool OnTouchEvent(TouchEvents event) override;
      };
    }

    template <>
    struct AppTraits<Apps::Calculator> {
      static constexpr Apps app = Apps::Calculator;
      static constexpr const char* icon = Screens::Symbols::calculator;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Calculator();
      }
    };
  }
}