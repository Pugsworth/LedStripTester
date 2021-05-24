#include <Arduino.h>
#include <JC_Button.h>
#include <RotaryEncoder.h>


namespace Menu
{
    template <class T>
    class MenuValue
    {
    public:
        T Value, Min, Max, Step;

        MenuValue(int min = 0, int max = 100, int step = 0) : Value(0), Min(min), Max(max), Step(step) {}
    };

    class MenuOption
    {
    public:
        const char *Text;
    };

    template <class T>
    class TypedMenuOption : public MenuOption
    {
    public:
        MenuValue<T> Value;

        TypedMenuOption(const char *text, MenuValue<T> mv) : Text(text), Value(mv) {}
    };

    MenuOption *Menu[] {
        new TypedMenuOption<int>("Int Option", {0, 255, 5}),
        new TypedMenuOption<float>("Double Option", {0.0f, 100.0f, 1.0f})
    };
}


void setup()
{

}

void loop()
{

}