// #include "display.h"

// #define EPD_2IN9D_WIDTH 128
// #define EPD_2IN9D_HEIGHT 296
// #define UWORD uint16_t

// Display::Display(int PIN_SPI_SCK, int PIN_SPI_DIN, int PIN_SPI_CS, int PIN_SPI_BUSY, int PIN_SPI_RST, int PIN_SPI_DC)
// {
//     PIN_SPI_SCK = PIN_SPI_SCK;
//     PIN_SPI_DIN = PIN_SPI_DIN;
//     PIN_SPI_CS = PIN_SPI_CS;
//     PIN_SPI_BUSY = PIN_SPI_BUSY;
//     PIN_SPI_RST = PIN_SPI_RST;
//     PIN_SPI_DC = PIN_SPI_DC;

//     pinMode(PIN_SPI_BUSY, INPUT);
//     pinMode(PIN_SPI_RST, OUTPUT);
//     pinMode(PIN_SPI_DC, OUTPUT);

//     pinMode(PIN_SPI_SCK, OUTPUT);
//     pinMode(PIN_SPI_DIN, OUTPUT);
//     pinMode(PIN_SPI_CS, OUTPUT);

//     digitalWrite(PIN_SPI_CS, HIGH);
//     digitalWrite(PIN_SPI_SCK, LOW);
//     EPD_Init_2in9d()
// }

// int Display::EPD_Init_2in9d()
// {
//     EPD_Reset();

//     EPD_SendCommand(0x01); //POWER SETTING
//     EPD_SendData(0x03);
//     EPD_SendData(0x00);
//     EPD_SendData(0x2b);
//     EPD_SendData(0x2b);
//     EPD_SendData(0x03);

//     EPD_SendCommand(0x06); //boost soft start
//     EPD_SendData(0x17);    //A
//     EPD_SendData(0x17);    //B
//     EPD_SendData(0x17);    //C

//     EPD_SendCommand(0x04);
//     EPD_2IN9D_ReadBusy();

//     EPD_SendCommand(0x00); //panel setting
//     EPD_SendData(0xbf);    //LUT from OTP，128x296
//     EPD_SendData(0x0e);    //VCOM to 0V fast

//     EPD_SendCommand(0x30); //PLL setting
//     EPD_SendData(0x3a);    // 3a 100HZ   29 150Hz 39 200HZ	31 171HZ

//     EPD_SendCommand(0x61); //resolution setting
//     EPD_SendData(EPD_2IN9D_WIDTH);
//     EPD_SendData((EPD_2IN9D_HEIGHT >> 8) & 0xff);
//     EPD_SendData(EPD_2IN9D_HEIGHT & 0xff);

//     EPD_SendCommand(0x82); //vcom_DC setting
//     EPD_SendData(0x28);
//     delay(2);

//     EPD_2IN9D_Clear();

//     return 0;
// }

// void Display::EPD_Reset()
// {
//     digitalWrite(PIN_SPI_RST, LOW);
//     delay(200);

//     digitalWrite(PIN_SPI_RST, HIGH);
//     delay(200);
// }

// void Display::EPD_SendCommand(byte command)
// {
//     digitalWrite(PIN_SPI_DC, LOW);
//     EpdSpiTransferCallback(command);
// }

// /* Sending a byte as a data --------------------------------------------------*/
// void Display::EPD_SendData(byte data)
// {
//     digitalWrite(PIN_SPI_DC, HIGH);
//     EpdSpiTransferCallback(data);
// }

// void Display::EpdSpiTransferCallback(byte data)
// {
//     //SPI.beginTransaction(spi_settings);
//     digitalWrite(PIN_SPI_CS, GPIO_PIN_RESET);

//     for (int i = 0; i < 8; i++)
//     {
//         if ((data & 0x80) == 0)
//             digitalWrite(PIN_SPI_DIN, GPIO_PIN_RESET);
//         else
//             digitalWrite(PIN_SPI_DIN, GPIO_PIN_SET);

//         data <<= 1;
//         digitalWrite(PIN_SPI_SCK, GPIO_PIN_SET);
//         digitalWrite(PIN_SPI_SCK, GPIO_PIN_RESET);
//     }

//     //SPI.transfer(data);
//     digitalWrite(PIN_SPI_CS, GPIO_PIN_SET);
//     //SPI.endTransaction();
// }

// void Display::EPD_2IN9D_ReadBusy(void)
// {
//     Serial.print("\r\ne-Paper busy");
//     //    UBYTE busy;
//     //    do {
//     //        EPD_SendCommand(0x71);
//     //        busy = digitalRead(PIN_SPI_BUSY);
//     //        busy = !(busy & 0x01);
//     //    } while(busy);
//     //    delay(200);
//     Serial.print("\r\ne-Paper busy free");
// }

// void Display::EPD_2IN9D_Clear(void)
// {
//     UWORD Width, Height;
//     Width = (EPD_2IN9D_WIDTH % 8 == 0) ? (EPD_2IN9D_WIDTH / 8) : (EPD_2IN9D_WIDTH / 8 + 1);
//     Height = EPD_2IN9D_HEIGHT;

//     EPD_SendCommand(0x10);
//     for (UWORD j = 0; j < Height; j++)
//     {
//         for (UWORD i = 0; i < Width; i++)
//         {
//             EPD_SendData(0x00);
//         }
//     }

//     EPD_SendCommand(0x13);
//     for (UWORD j = 0; j < Height; j++)
//     {
//         for (UWORD i = 0; i < Width; i++)
//         {
//             EPD_SendData(0xFF);
//         }
//     }

//     // EPD_2IN9D_Show();
//     EPD_SendCommand(0x13);
// }

// Display::~Display()
// {
// }
