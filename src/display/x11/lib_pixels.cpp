/*********************************************************************************************************************
** CHUNKY32
*/

static void VideoDrawPixel32(objBitmap *Bitmap, int X, int Y, uint32_t Colour)
{
   XSetForeground(XDisplay, ((extBitmap *)Bitmap)->getGC(), Colour);
   XDrawPoint(XDisplay, ((extBitmap *)Bitmap)->x11.drawable, ((extBitmap *)Bitmap)->getGC(), X, Y);
}

static void VideoDrawRGBPixel32(objBitmap *Bitmap, int X, int Y, RGB8 *RGB)
{
   XSetForeground(XDisplay, ((extBitmap *)Bitmap)->getGC(), Bitmap->packPixelWB(*RGB));
   XDrawPoint(XDisplay, ((extBitmap *)Bitmap)->x11.drawable, ((extBitmap *)Bitmap)->getGC(), X, Y);
}

static void VideoDrawRGBIndex32(objBitmap *Bitmap, uint32_t *Data, RGB8 *RGB)
{

}

static uint32_t VideoReadPixel32(objBitmap *Bitmap, int X, int Y)
{
   return ((uint32_t *)((uint8_t *)((extBitmap *)Bitmap)->x11.readable->data + (((extBitmap *)Bitmap)->x11.readable->bytes_per_line * Y) + (X<<2)))[0];
}

static void VideoReadRGBPixel32(objBitmap *Bitmap, int X, int Y, RGB8 *RGB)
{
   uint32_t colour = ((uint32_t *)((uint8_t *)((extBitmap *)Bitmap)->x11.readable->data + (((extBitmap *)Bitmap)->x11.readable->bytes_per_line * Y) + (X<<2)))[0];
   RGB->Red   = (uint8_t)(colour >> ((extBitmap *)Bitmap)->prvColourFormat.RedPos);
   RGB->Green = (uint8_t)(colour >> ((extBitmap *)Bitmap)->prvColourFormat.GreenPos);
   RGB->Blue  = (uint8_t)(colour >> ((extBitmap *)Bitmap)->prvColourFormat.BluePos);
   RGB->Alpha = (uint8_t)(colour >> ((extBitmap *)Bitmap)->prvColourFormat.AlphaPos);
}

static void VideoReadRGBIndex32(objBitmap *Bitmap, uint32_t *Data, RGB8 *RGB)
{
   uint32_t colour = Data[0];
   RGB->Red   = (uint8_t)(colour >> ((extBitmap *)Bitmap)->prvColourFormat.RedPos);
   RGB->Green = (uint8_t)(colour >> ((extBitmap *)Bitmap)->prvColourFormat.GreenPos);
   RGB->Blue  = (uint8_t)(colour >> ((extBitmap *)Bitmap)->prvColourFormat.BluePos);
   RGB->Alpha = (uint8_t)(colour >> ((extBitmap *)Bitmap)->prvColourFormat.AlphaPos);
}

/*********************************************************************************************************************
** CHUNKY24
*/

static void VideoDrawPixel24(objBitmap *Bitmap, int X, int Y, uint32_t Colour)
{
   XSetForeground(XDisplay, ((extBitmap *)Bitmap)->getGC(), Colour);
   XDrawPoint(XDisplay, ((extBitmap *)Bitmap)->x11.drawable, ((extBitmap *)Bitmap)->getGC(), X, Y);
}

static void VideoDrawRGBPixel24(objBitmap *Bitmap, int X, int Y, RGB8 *RGB)
{
   uint32_t Colour = (RGB->Red<<16) | (RGB->Green<<8) | (RGB->Blue);
   XSetForeground(XDisplay, ((extBitmap *)Bitmap)->getGC(), Colour);
   XDrawPoint(XDisplay, ((extBitmap *)Bitmap)->x11.drawable, ((extBitmap *)Bitmap)->getGC(), X, Y);
}

static void VideoDrawRGBIndex24(objBitmap *Bitmap, uint8_t *Data, RGB8 *RGB)
{

}

static uint32_t VideoReadPixel24(objBitmap *Bitmap, int X, int Y)
{
   auto data = (uint8_t *)((extBitmap *)Bitmap)->x11.readable->data + (Bitmap->LineWidth * Y) + (X + X + X);
   return (data[2]<<16)|(data[1]<<8)|data[0];
}

static void VideoReadRGBPixel24(objBitmap *Bitmap, int X, int Y, RGB8 *RGB)
{
   auto data = (uint8_t *)((extBitmap *)Bitmap)->x11.readable->data;
   data += ((extBitmap *)Bitmap)->x11.readable->bytes_per_line * Y;
   data += X + X + X;
   RGB->Red   = data[2];
   RGB->Green = data[1];
   RGB->Blue  = data[0];
   RGB->Alpha = 0;
}

static void VideoReadRGBIndex24(objBitmap *Bitmap, uint8_t *Data, RGB8 *RGB)
{
   RGB->Red   = Data[2];
   RGB->Green = Data[1];
   RGB->Blue  = Data[0];
   RGB->Alpha = 0;
}

/*********************************************************************************************************************
** CHUNKY16
*/

static void VideoDrawPixel16(objBitmap *Bitmap, int X, int Y, uint32_t Colour)
{
   XSetForeground(XDisplay, ((extBitmap *)Bitmap)->getGC(), Colour);
   XDrawPoint(XDisplay, ((extBitmap *)Bitmap)->x11.drawable, ((extBitmap *)Bitmap)->getGC(), X, Y);
}

static void VideoDrawRGBPixel16(objBitmap *Bitmap, int X, int Y, RGB8 *RGB)
{
   XSetForeground(XDisplay, ((extBitmap *)Bitmap)->getGC(), Bitmap->packPixel(*RGB));
   XDrawPoint(XDisplay, ((extBitmap *)Bitmap)->x11.drawable, ((extBitmap *)Bitmap)->getGC(), X, Y);
}

static void VideoDrawRGBIndex16(objBitmap *Bitmap, uint16_t *Data, RGB8 *RGB)
{

}

static uint32_t VideoReadPixel16(objBitmap *Bitmap, int X, int Y)
{
   return ((uint16_t *)((int8_t *)((extBitmap *)Bitmap)->x11.readable->data + (((extBitmap *)Bitmap)->x11.readable->bytes_per_line * Y) + (X<<1)))[0];
}

static void VideoReadRGBPixel16(objBitmap *Bitmap, int X, int Y, RGB8 *RGB)
{
   uint16_t data = ((uint16_t *)((int8_t *)((extBitmap *)Bitmap)->x11.readable->data + (((extBitmap *)Bitmap)->x11.readable->bytes_per_line * Y) + (X<<1)))[0];
   RGB->Red   = Bitmap->unpackRed(data);
   RGB->Green = Bitmap->unpackGreen(data);
   RGB->Blue  = Bitmap->unpackBlue(data);
   RGB->Alpha = 0;
}

static void VideoReadRGBIndex16(objBitmap *Bitmap, uint16_t *Data, RGB8 *RGB)
{
   RGB->Red   = Bitmap->unpackRed(Data[0]);
   RGB->Green = Bitmap->unpackGreen(Data[0]);
   RGB->Blue  = Bitmap->unpackBlue(Data[0]);
   RGB->Alpha = 0;
}

/*********************************************************************************************************************
** CHUNKY8
*/

static void VideoDrawPixel8(objBitmap *Bitmap, int X, int Y, uint32_t Colour)
{
   XDrawPoint(XDisplay, ((extBitmap *)Bitmap)->x11.drawable, ((extBitmap *)Bitmap)->getGC(), X, Y);
}

static void VideoDrawRGBPixel8(objBitmap *Bitmap, int X, int Y, RGB8 *RGB)
{
   //ULONG colour = RGBToValue(RGB, Bitmap->Palette);
   XDrawPoint(XDisplay, ((extBitmap *)Bitmap)->x11.drawable, ((extBitmap *)Bitmap)->getGC(), X, Y);
}

static void VideoDrawRGBIndex8(objBitmap *Bitmap, uint8_t *Data, RGB8 *RGB)
{

}

static uint32_t VideoReadPixel8(objBitmap *Bitmap, int X, int Y)
{
   return (((extBitmap *)Bitmap)->x11.readable->data + (((extBitmap *)Bitmap)->x11.readable->bytes_per_line * Y) + X)[0];
}

static void VideoReadRGBPixel8(objBitmap *Bitmap, int X, int Y, RGB8 *RGB)
{
   auto data  = (uint8_t *)((extBitmap *)Bitmap)->x11.readable->data;
   auto index = data[(((extBitmap *)Bitmap)->x11.readable->bytes_per_line * Y) + X];
   RGB->Red   = Bitmap->Palette->Col[index].Red;
   RGB->Green = Bitmap->Palette->Col[index].Green;
   RGB->Blue  = Bitmap->Palette->Col[index].Blue;
   RGB->Alpha = 0;
}

static void VideoReadRGBIndex8(objBitmap *Bitmap, uint8_t *Data, RGB8 *RGB)
{
   RGB->Red   = Bitmap->Palette->Col[*Data].Red;
   RGB->Green = Bitmap->Palette->Col[*Data].Green;
   RGB->Blue  = Bitmap->Palette->Col[*Data].Blue;
   RGB->Alpha = 0;
}
