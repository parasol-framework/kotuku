/*********************************************************************************************************************
** CHUNKY32
*/

static void VideoDrawPixel32(objBitmap *Bitmap, int X, int Y, uint32_t Colour)
{
}

static void VideoDrawRGBPixel32(objBitmap *Bitmap, int X, int Y, struct RGB8 *RGB)
{
}

static void VideoDrawRGBIndex32(objBitmap *Bitmap, uint32_t *Data, struct RGB8 *RGB)
{

}

static uint32_t VideoReadPixel32(objBitmap *Bitmap, int X, int Y)
{
   return 0;
}

static void VideoReadRGBPixel32(objBitmap *Bitmap, int X, int Y, struct RGB8 *RGB)
{
}

static void VideoReadRGBIndex32(objBitmap *Bitmap, uint32_t *Data, struct RGB8 *RGB)
{
}

/*********************************************************************************************************************
** CHUNKY24
*/

static void VideoDrawPixel24(objBitmap *Bitmap, int X, int Y, uint32_t Colour)
{
}

static void VideoDrawRGBPixel24(objBitmap *Bitmap, int X, int Y, struct RGB8 *RGB)
{
}

static void VideoDrawRGBIndex24(objBitmap *Bitmap, uint8_t *Data, struct RGB8 *RGB)
{

}

static uint32_t VideoReadPixel24(objBitmap *Bitmap, int X, int Y)
{
   return 0;
}

static void VideoReadRGBPixel24(objBitmap *Bitmap, int X, int Y, struct RGB8 *RGB)
{

}

static void VideoReadRGBIndex24(objBitmap *Bitmap, uint8_t *Data, struct RGB8 *RGB)
{
}

/*********************************************************************************************************************
** CHUNKY16
*/

static void VideoDrawPixel16(objBitmap *Bitmap, int X, int Y, uint32_t Colour)
{
}

static void VideoDrawRGBPixel16(objBitmap *Bitmap, int X, int Y, struct RGB8 *RGB)
{
}

static void VideoDrawRGBIndex16(objBitmap *Bitmap, uint16_t *Data, struct RGB*RGB)
{

}

static uint32_t VideoReadPixel16(objBitmap *Bitmap, int X, int Y)
{
   return 0;
}

static void VideoReadRGBPixel16(objBitmap *Bitmap, int X, int Y, struct RGB8 *RGB)
{

}

static void VideoReadRGBIndex16(objBitmap *Bitmap, uint16_t *Data, struct RGB*RGB)
{
}

/*********************************************************************************************************************
** CHUNKY8
*/

static void VideoDrawPixel8(objBitmap *Bitmap, int X, int Y, uint32_t Colour)
{
}

static void VideoDrawRGBPixel8(objBitmap *Bitmap, int X, int Y, struct RGB8 *RGB)
{
}

static void VideoDrawRGBIndex8(objBitmap *Bitmap, uint8_t *Data, struct RGB8 *RGB)
{

}

static uint32_t VideoReadPixel8(objBitmap *Bitmap, int X, int Y)
{
   return 0;
}

static void VideoReadRGBPixel8(objBitmap *Bitmap, int X, int Y, struct RGB8 *RGB)
{
}

static void VideoReadRGBIndex8(objBitmap *Bitmap, uint8_t *Data, struct RGB*RGB)
{
   RGB->Red   = Bitmap->Palette->Col[*Data].Red;
   RGB->Green = Bitmap->Palette->Col[*Data].Green;
   RGB->Blue  = Bitmap->Palette->Col[*Data].Blue;
   RGB->Alpha = 0;
}
