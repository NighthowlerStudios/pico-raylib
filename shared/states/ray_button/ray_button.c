#include "ray_button.h"
#include "pio_button.h"

void SetupAllButtons(void)
{
    for (int i = 0; i < numButtonsToTest; i++)
    {
        SetupButton(picoButtonTable[i].buttonPin);
    }
}

void PollAllButtons(void)
{
    for (int i = 0; i < numButtonsToTest; i++)
    {
        // These buttons go low when pressed, not high.
        picoButtonTable[i].isDown = IsButtonDown(picoButtonTable[i].buttonPin);
    }
}