class Display
{
private:
    int PIN_SPI_SCK;
    int PIN_SPI_DIN;
    int PIN_SPI_CS;
    int PIN_SPI_BUSY;
    int PIN_SPI_RST;
    int PIN_SPI_DC;
    int EPD_Init_2in9d();
    void EPD_Reset();
    void EPD_SendCommand(byte command);
    void EPD_2IN9D_ReadBusy(void);
    void EPD_2IN9D_Clear(void);
    void EpdSpiTransferCallback(byte data);
    void EPD_SetFullReg(void);

public:
    Display(int sck, int din, int cs, int busy, int rst, int dc);
    void EPD_2IN9D_Show(void);
    void EPD_SendData(byte data);
    ~Display();
};

#define EPD_2IN9D_WIDTH 128
#define EPD_2IN9D_HEIGHT 296
#define UWORD uint16_t
#define GPIO_PIN_SET 1
#define GPIO_PIN_RESET 0

Display::Display(int sck, int din, int cs, int busy, int rst, int dc)
{
    PIN_SPI_SCK = sck;
    PIN_SPI_DIN = din;
    PIN_SPI_CS = cs;
    PIN_SPI_BUSY = busy;
    PIN_SPI_RST = rst;
    PIN_SPI_DC = dc;

    pinMode(PIN_SPI_BUSY, INPUT);
    pinMode(PIN_SPI_RST, OUTPUT);
    pinMode(PIN_SPI_DC, OUTPUT);

    pinMode(PIN_SPI_SCK, OUTPUT);
    pinMode(PIN_SPI_DIN, OUTPUT);
    pinMode(PIN_SPI_CS, OUTPUT);

    digitalWrite(PIN_SPI_CS, HIGH);
    digitalWrite(PIN_SPI_SCK, LOW);
    EPD_Init_2in9d();
}

int Display::EPD_Init_2in9d()
{
    EPD_Reset();

    EPD_SendCommand(0x01); //POWER SETTING
    EPD_SendData(0x03);
    EPD_SendData(0x00);
    EPD_SendData(0x2b);
    EPD_SendData(0x2b);
    EPD_SendData(0x03);

    EPD_SendCommand(0x06); //boost soft start
    EPD_SendData(0x17);    //A
    EPD_SendData(0x17);    //B
    EPD_SendData(0x17);    //C

    EPD_SendCommand(0x04);
    EPD_2IN9D_ReadBusy();

    EPD_SendCommand(0x00); //panel setting
    EPD_SendData(0xbf);    //LUT from OTP，128x296
    EPD_SendData(0x0e);    //VCOM to 0V fast

    EPD_SendCommand(0x30); //PLL setting
    EPD_SendData(0x3a);    // 3a 100HZ   29 150Hz 39 200HZ	31 171HZ

    EPD_SendCommand(0x61); //resolution setting
    EPD_SendData(EPD_2IN9D_WIDTH);
    EPD_SendData((EPD_2IN9D_HEIGHT >> 8) & 0xff);
    EPD_SendData(EPD_2IN9D_HEIGHT & 0xff);

    EPD_SendCommand(0x82); //vcom_DC setting
    EPD_SendData(0x28);
    delay(2);

    EPD_2IN9D_Clear();

    return 0;
}

void Display::EPD_Reset()
{
    digitalWrite(PIN_SPI_RST, LOW);
    delay(200);

    digitalWrite(PIN_SPI_RST, HIGH);
    delay(200);
}

void Display::EPD_SendCommand(byte command)
{
    digitalWrite(PIN_SPI_DC, LOW);
    EpdSpiTransferCallback(command);
}

/* Sending a byte as a data --------------------------------------------------*/
void Display::EPD_SendData(byte data)
{
    digitalWrite(PIN_SPI_DC, HIGH);
    EpdSpiTransferCallback(data);
}

void Display::EpdSpiTransferCallback(byte data)
{
    //SPI.beginTransaction(spi_settings);
    digitalWrite(PIN_SPI_CS, GPIO_PIN_RESET);

    for (int i = 0; i < 8; i++)
    {
        if ((data & 0x80) == 0)
            digitalWrite(PIN_SPI_DIN, GPIO_PIN_RESET);
        else
            digitalWrite(PIN_SPI_DIN, GPIO_PIN_SET);

        data <<= 1;
        digitalWrite(PIN_SPI_SCK, GPIO_PIN_SET);
        digitalWrite(PIN_SPI_SCK, GPIO_PIN_RESET);
    }

    //SPI.transfer(data);
    digitalWrite(PIN_SPI_CS, GPIO_PIN_SET);
    //SPI.endTransaction();
}

void Display::EPD_2IN9D_ReadBusy(void)
{
    Serial.print("\r\ne-Paper busy");
    //    UBYTE busy;
    //    do {
    //        EPD_SendCommand(0x71);
    //        busy = digitalRead(PIN_SPI_BUSY);
    //        busy = !(busy & 0x01);
    //    } while(busy);
    //    delay(200);
    Serial.print("\r\ne-Paper busy free");
}

void Display::EPD_2IN9D_Clear(void)
{
    UWORD Width, Height;
    Width = (EPD_2IN9D_WIDTH % 8 == 0) ? (EPD_2IN9D_WIDTH / 8) : (EPD_2IN9D_WIDTH / 8 + 1);
    Height = EPD_2IN9D_HEIGHT;

    EPD_SendCommand(0x10);
    for (UWORD j = 0; j < Height; j++)
    {
        for (UWORD i = 0; i < Width; i++)
        {
            EPD_SendData(0x00);
        }
    }

    EPD_SendCommand(0x13);
    for (UWORD j = 0; j < Height; j++)
    {
        for (UWORD i = 0; i < Width; i++)
        {
            EPD_SendData(0xFF);
        }
    }

    // EPD_2IN9D_Show();
    EPD_SendCommand(0x13);
}

void Display::EPD_2IN9D_Show(void)
{
    Serial.print("\r\nEPD_2IN9D_Show");
    EPD_SetFullReg();
    EPD_SendCommand(0x12); //DISPLAY REFRESH
    delay(10);             //!!!The delay here is necessary, 200uS at least!!!

    EPD_2IN9D_ReadBusy();

    // Sleep
    EPD_SendCommand(0X50);
    EPD_SendData(0xf7);
    EPD_SendCommand(0X02); //power off
    EPD_2IN9D_ReadBusy();
    EPD_SendCommand(0X07); //deep sleep
    EPD_SendData(0xA5);
}

const unsigned char EPD_2IN9D_lut_vcomDC[] = {
    0x00, 0x08, 0x00, 0x00, 0x00, 0x02,
    0x60, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x00, 0x14, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00,
};
const unsigned char EPD_2IN9D_lut_ww[] = {
    0x40, 0x08, 0x00, 0x00, 0x00, 0x02,
    0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x40, 0x14, 0x00, 0x00, 0x00, 0x01,
    0xA0, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
const unsigned char EPD_2IN9D_lut_bw[] = {
    0x40, 0x17, 0x00, 0x00, 0x00, 0x02,
    0x90, 0x0F, 0x0F, 0x00, 0x00, 0x03,
    0x40, 0x0A, 0x01, 0x00, 0x00, 0x01,
    0xA0, 0x0E, 0x0E, 0x00, 0x00, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
const unsigned char EPD_2IN9D_lut_wb[] = {
    0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
    0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x80, 0x14, 0x00, 0x00, 0x00, 0x01,
    0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
const unsigned char EPD_2IN9D_lut_bb[] = {
    0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
    0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x80, 0x14, 0x00, 0x00, 0x00, 0x01,
    0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

void Display::EPD_SetFullReg(void)
{
    EPD_SendCommand(0X50); //VCOM AND DATA INTERVAL SETTING
    EPD_SendData(0xb7);    //WBmode:VBDF 17|D7 VBDW 97 VBDB 57		WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7

    unsigned int count;
    EPD_SendCommand(0x20);
    for (count = 0; count < 44; count++)
    {
        EPD_SendData(EPD_2IN9D_lut_vcomDC[count]);
    }

    EPD_SendCommand(0x21);
    for (count = 0; count < 42; count++)
    {
        EPD_SendData(EPD_2IN9D_lut_ww[count]);
    }

    EPD_SendCommand(0x22);
    for (count = 0; count < 42; count++)
    {
        EPD_SendData(EPD_2IN9D_lut_bw[count]);
    }

    EPD_SendCommand(0x23);
    for (count = 0; count < 42; count++)
    {
        EPD_SendData(EPD_2IN9D_lut_wb[count]);
    }

    EPD_SendCommand(0x24);
    for (count = 0; count < 42; count++)
    {
        EPD_SendData(EPD_2IN9D_lut_bb[count]);
    }
}

Display::~Display()
{
}
