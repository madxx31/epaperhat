#define EPD_2IN9D_WIDTH 128
#define EPD_2IN9D_HEIGHT 296
#define UWORD uint16_t
#define GPIO_PIN_SET 1
#define GPIO_PIN_RESET 0

enum align
{
    CENTER,
    RIGHT,
    LEFT,
    WIDTH
};

int spacing = 10;
const uint8_t symbol_widths[] = {0, 11, 18, 32, 30, 38, 37, 8, 17, 18, 31, 28, 12, 17, 12, 23, 30, 19, 30, 29, 31, 30, 29, 30, 28, 29, 12, 14, 25, 26, 25, 27, 48, 41, 32, 34, 33, 29, 28, 34, 35, 11, 29, 35, 28, 45, 34, 35, 33, 37, 33, 33, 35, 33, 39, 49, 36, 37, 32, 14, 27, 14, 25, 27, 18, 18, 7, 18, 32, 29, 41, 33, 32, 29, 43, 29, 59, 32, 35, 35, 36, 37, 45, 35, 35, 35, 33, 34, 35, 35, 45, 36, 41, 34, 50, 56, 43, 44, 33, 35, 49, 34, 53};

int get_width(uint8_t *data, size_t len)
{
    int total_width = 0;
    for (int i = 0; i < len; ++i)
    {
        total_width += symbol_widths[data[i]];
    }
    total_width += ((len - 1) * spacing);
    return total_width;
}

class Display
{
private:
    uint8_t PIN_SPI_SCK;
    uint8_t PIN_SPI_DIN;
    uint8_t PIN_SPI_CS;
    uint8_t PIN_SPI_BUSY;
    uint8_t PIN_SPI_RST;
    uint8_t PIN_SPI_DC;

    void reset();
    void send_command(byte command);
    const void send_data(byte data);
    void clear(void);
    void spi_transfer_callback(byte data);
    void set_full_reg(void);

public:
    Display(uint8_t sck, uint8_t din, uint8_t cs, uint8_t busy, uint8_t rst, uint8_t dc);
    int init();
    const void show(void);
    void fill(int width);
    void display_clear();
    void display_symbol(uint8_t symbol_id);
    void display_text(uint8_t *text, size_t len, align a);
    ~Display();
};

Display::Display(uint8_t sck, uint8_t din, uint8_t cs, uint8_t busy, uint8_t rst, uint8_t dc)
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
}

int Display::init()
{
    reset();

    send_command(0x01); //POWER SETTING
    send_data(0x03);
    send_data(0x00);
    send_data(0x2b);
    send_data(0x2b);
    send_data(0x03);

    send_command(0x06); //boost soft start
    send_data(0x17);    //A
    send_data(0x17);    //B
    send_data(0x17);    //C

    send_command(0x04);

    send_command(0x00); //panel setting
    send_data(0xbf);    //LUT from OTP，128x296
    send_data(0x0e);    //VCOM to 0V fast

    send_command(0x30); //PLL setting
    send_data(0x3a);    // 3a 100HZ   29 150Hz 39 200HZ	31 171HZ

    send_command(0x61); //resolution setting
    send_data(EPD_2IN9D_WIDTH);
    send_data((EPD_2IN9D_HEIGHT >> 8) & 0xff);
    send_data(EPD_2IN9D_HEIGHT & 0xff);

    send_command(0x82); //vcom_DC setting
    send_data(0x28);
    delay(2);

    clear();

    return 0;
}

void Display::reset()
{
    digitalWrite(PIN_SPI_RST, LOW);
    delay(200);

    digitalWrite(PIN_SPI_RST, HIGH);
    delay(200);
}

void Display::send_command(byte command)
{
    digitalWrite(PIN_SPI_DC, LOW);
    spi_transfer_callback(command);
}

/* Sending a byte as a data --------------------------------------------------*/
const void Display::send_data(byte data)
{
    digitalWrite(PIN_SPI_DC, HIGH);
    spi_transfer_callback(data);
}

void Display::spi_transfer_callback(byte data)
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

void Display::clear(void)
{
    UWORD Width, Height;
    Width = (EPD_2IN9D_WIDTH % 8 == 0) ? (EPD_2IN9D_WIDTH / 8) : (EPD_2IN9D_WIDTH / 8 + 1);
    Height = EPD_2IN9D_HEIGHT;

    send_command(0x10);
    for (UWORD j = 0; j < Height; j++)
    {
        for (UWORD i = 0; i < Width; i++)
        {
            send_data(0x00);
        }
    }

    send_command(0x13);
    for (UWORD j = 0; j < Height; j++)
    {
        for (UWORD i = 0; i < Width; i++)
        {
            send_data(0xFF);
        }
    }

    // EPD_2IN9D_Show();
    send_command(0x13);
}

const void Display::show(void)
{
    set_full_reg();
    send_command(0x12); //DISPLAY REFRESH
    delay(10);          //!!!The delay here is necessary, 200uS at least!!!


    // Sleep
    send_command(0X50);
    send_data(0xf7);
    send_command(0X02); //power off
    send_command(0X07); //deep sleep
    send_data(0xA5);
}

const unsigned char EPD_2IN9D_lut_vcomDC[] = {
    0x00,
    0x08,
    0x00,
    0x00,
    0x00,
    0x02,
    0x60,
    0x28,
    0x28,
    0x00,
    0x00,
    0x01,
    0x00,
    0x14,
    0x00,
    0x00,
    0x00,
    0x01,
    0x00,
    0x12,
    0x12,
    0x00,
    0x00,
    0x01,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};
const unsigned char EPD_2IN9D_lut_ww[] = {
    0x40,
    0x08,
    0x00,
    0x00,
    0x00,
    0x02,
    0x90,
    0x28,
    0x28,
    0x00,
    0x00,
    0x01,
    0x40,
    0x14,
    0x00,
    0x00,
    0x00,
    0x01,
    0xA0,
    0x12,
    0x12,
    0x00,
    0x00,
    0x01,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};
const unsigned char EPD_2IN9D_lut_bw[] = {
    0x40,
    0x17,
    0x00,
    0x00,
    0x00,
    0x02,
    0x90,
    0x0F,
    0x0F,
    0x00,
    0x00,
    0x03,
    0x40,
    0x0A,
    0x01,
    0x00,
    0x00,
    0x01,
    0xA0,
    0x0E,
    0x0E,
    0x00,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};
const unsigned char EPD_2IN9D_lut_wb[] = {
    0x80,
    0x08,
    0x00,
    0x00,
    0x00,
    0x02,
    0x90,
    0x28,
    0x28,
    0x00,
    0x00,
    0x01,
    0x80,
    0x14,
    0x00,
    0x00,
    0x00,
    0x01,
    0x50,
    0x12,
    0x12,
    0x00,
    0x00,
    0x01,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};
const unsigned char EPD_2IN9D_lut_bb[] = {
    0x80,
    0x08,
    0x00,
    0x00,
    0x00,
    0x02,
    0x90,
    0x28,
    0x28,
    0x00,
    0x00,
    0x01,
    0x80,
    0x14,
    0x00,
    0x00,
    0x00,
    0x01,
    0x50,
    0x12,
    0x12,
    0x00,
    0x00,
    0x01,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};

void Display::set_full_reg(void)
{
    send_command(0X50); //VCOM AND DATA INTERVAL SETTING
    send_data(0xb7);    //WBmode:VBDF 17|D7 VBDW 97 VBDB 57		WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7

    unsigned int count;
    send_command(0x20);
    for (count = 0; count < 44; count++)
    {
        send_data(EPD_2IN9D_lut_vcomDC[count]);
    }

    send_command(0x21);
    for (count = 0; count < 42; count++)
    {
        send_data(EPD_2IN9D_lut_ww[count]);
    }

    send_command(0x22);
    for (count = 0; count < 42; count++)
    {
        send_data(EPD_2IN9D_lut_bw[count]);
    }

    send_command(0x23);
    for (count = 0; count < 42; count++)
    {
        send_data(EPD_2IN9D_lut_wb[count]);
    }

    send_command(0x24);
    for (count = 0; count < 42; count++)
    {
        send_data(EPD_2IN9D_lut_bb[count]);
    }
}

void Display::fill(int width)
{
    for (size_t i = 0; i < 128 * width / 8; i++)
    {
        send_data((byte)0);
    }
}

void Display::display_symbol(uint8_t symbol_id)
{
    File f = SPIFFS.open("/font/" + String(symbol_id) + ".bin");
    int w = symbol_widths[symbol_id];
    for (size_t i = 0; i < 128 * w / 8; i++)
    {
        send_data((byte)f.read());
    }
    f.close();
}

void Display::display_text(uint8_t *text, size_t len, align a)
{
    init();
    int total_width = get_width(text, len);
    switch (a)
    {
    case CENTER:
    {
        fill(ceil((296 - total_width) / 2));
        for (int i = 0; i < len; ++i)
        {
            display_symbol(text[len - i - 1]);
            if (i != len - 1)
                fill(spacing);
        }
        fill(floor((296 - total_width) / 2));
        break;
    }
    case LEFT:
    {
        fill(floor(296 - total_width));
        for (int i = 0; i < len; ++i)
        {
            display_symbol(text[len - i - 1]);
            if (i != len - 1)
                fill(spacing);
        }
        break;
    }
    case RIGHT:
    {
        for (int i = 0; i < len; ++i)
        {
            display_symbol(text[len - i - 1]);
            if (i != len - 1)
                fill(spacing);
        }
        fill(floor(296 - total_width));
        break;
    }
    case WIDTH:
    {
        int spread = 296 - total_width;
        int n_spaces = 0;
        for (int i = 0; i < len; ++i)
        {
            if (text[i] == 0)
                n_spaces++;
        }

        for (int i = 0; i < len; ++i)
        {
            display_symbol(text[len - i - 1]);
            if (i != len - 1)
            {
                fill(spacing);
                if (n_spaces == 0)
                {
                    int add = spread / (len - i - 1);
                    spread -= add;
                    fill(add);
                }
            }
            if (text[len - i - 1] == 0)
            {
                int add = spread / n_spaces;
                spread -= add;
                n_spaces -= 1;
                fill(add);
            }
        }
        break;
    }
    }
    show();
}

void Display::display_clear()
{
    init();
    fill(296);
    show();
}

Display::~Display()
{
}
