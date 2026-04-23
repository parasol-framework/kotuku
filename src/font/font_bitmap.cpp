
/******************************************************************************
** Win32 font structures
*/

struct winFont {
   int Offset, Size, Point;
};

struct winmz_header_fields {
   uint16_t magic;
   uint8_t data[29 * 2];
   uint32_t lfanew;
};

struct winne_header_fields {
   uint16_t magic;
   uint8_t data[34];
   uint16_t resource_tab_offset;
   uint16_t rname_tab_offset;
};

PACK(struct winfnt_header_fields {
   uint16_t version;
   uint32_t file_size;
   char copyright[60];
   uint16_t file_type;
   uint16_t nominal_point_size;     // Point size
   uint16_t vertical_resolution;
   uint16_t horizontal_resolution;
   uint16_t ascent;                 // The amount of pixels above the base-line
   uint16_t internal_leading;       // top leading pixels
   uint16_t external_leading;       // gutter
   int8_t  italic;                 // TRUE if font is italic
   int8_t  underline;              // TRUE if font is underlined
   int8_t  strike_out;             // TRUE if font is striked-out
   uint16_t weight;                 // Indicates font boldness
   int8_t  charset;
   uint16_t pixel_width;
   uint16_t pixel_height;
   int8_t  pitch_and_family;
   uint16_t avg_width;
   uint16_t max_width;
   uint8_t first_char;
   uint8_t last_char;
   uint8_t default_char;
   uint8_t break_char;
   uint16_t bytes_per_row;
   uint32_t device_offset;
   uint32_t face_name_offset;
   uint32_t bits_pointer;
   uint32_t bits_offset;
   int8_t  reserved;
   uint32_t flags;
   uint16_t A_space;
   uint16_t B_space;
   uint16_t C_space;
   uint16_t color_table_offset;
   int8_t  reservedend[4];
});

#define ID_WINMZ  0x5A4D
#define ID_WINNE  0x454E

//*****************************************************************************
// Structure definition for cached bitmap fonts.

class BitmapCache {
private:
   uint8_t *mOutline;

public:
   uint8_t *mData;
   winfnt_header_fields Header;
   FontCharacter Chars[256];
   std::string Path;
   int16_t OpenCount;
   FTF StyleFlags;
   ERR Result;

   BitmapCache(winfnt_header_fields &pFace, CSTRING pStyle, CSTRING pPath, objFile *pFile, winFont &pWinFont) {
      pf::Log log(__FUNCTION__);

      log.branch("Caching font %s : %d : %s", pPath, pFace.nominal_point_size, pStyle);

      mData     = nullptr;
      mOutline  = nullptr;
      OpenCount = 0;
      Result    = ERR::Okay;
      Header    = pFace;

      if (iequals("Bold", pStyle)) StyleFlags = FTF::BOLD;
      else if (iequals("Italic", pStyle)) StyleFlags = FTF::ITALIC;
      else if (iequals("Bold Italic", pStyle)) StyleFlags = FTF::BOLD|FTF::ITALIC;
      else StyleFlags = FTF::NIL;

      Path = pPath;

      // Read character information from the file

      pFile->seek(pWinFont.Offset + 118, SEEK::START);

      clearmem(Chars, sizeof(Chars));
      if (pFace.version IS 0x300) {
         int j = pFace.first_char;
         for (int i=0; i < pFace.last_char - pFace.first_char + 1; i++) {
            uint16_t width;
            uint32_t offset;

            if (fl::ReadLE(pFile, &width) != ERR::Okay) break;
            if (fl::ReadLE(pFile, &offset) != ERR::Okay) break;

            Chars[j].Width   = width;
            Chars[j].Advance = Chars[j].Width;
            Chars[j].Offset  = offset - pFace.bits_offset;
            j++;
         }
      }
      else {
         int j = pFace.first_char;
         for (int i=0; i < pFace.last_char - pFace.first_char + 1; i++) {
            uint16_t width, offset;
            if (fl::ReadLE(pFile, &width) != ERR::Okay) break;
            if (fl::ReadLE(pFile, &offset) != ERR::Okay) break;
            Chars[j].Width   = width;
            Chars[j].Advance = Chars[j].Width;
            Chars[j].Offset  = offset - pFace.bits_offset;
            j++;
         }
      }

      int size = pFace.file_size - pFace.bits_offset;

      if (AllocMemory(size, MEM::UNTRACKED, &mData) IS ERR::Okay) {
         int result;
         pFile->seek(pWinFont.Offset + pFace.bits_offset, SEEK::START);

         if ((pFile->read(mData, size, &result) IS ERR::Okay) and (result IS size)) {
            // Convert the graphics format for wide characters from column-first format to row-first format.

            for (int16_t i=0; i < 256; i++) {
               if (!Chars[i].Width) continue;

               int sz = ((Chars[i].Width+7)>>3) * pFace.pixel_height;
               if (Chars[i].Width > 8) {
                  auto buffer = std::make_unique<uint8_t[]>(sz);
                  clearmem(buffer.get(), sz);

                  uint8_t *gfx = mData + Chars[i].Offset;
                  int bytewidth = (Chars[i].Width + 7)>>3;
                  int pos = 0;
                  for (int k=0; k < pFace.pixel_height; k++) {
                     for (int j=0; j < bytewidth; j++) {
                        buffer[pos++] = gfx[k + (j * pFace.pixel_height)];
                     }
                  }

                  copymem(buffer.get(), gfx, pos);
               }
            }
         }
         else Result = log.warning(ERR::Read);
      }
      else Result = log.warning(ERR::AllocMemory);

      if (((StyleFlags & FTF::BOLD) != FTF::NIL) and (Header.weight < 600)) {
         log.msg("Converting base font graphics data to bold.");

         int size = 0;
         for (int i=0; i < 256; i++) {
            if (Chars[i].Width) size += Header.pixel_height * ((Chars[i].Width+8)>>3);
         }

         uint8_t *buffer;
         if (AllocMemory(size, MEM::UNTRACKED, &buffer) IS ERR::Okay) {
            int pos = 0;
            for (int i=0; i < 256; i++) {
               if (Chars[i].Width) {
                  uint8_t *gfx = mData + Chars[i].Offset;
                  Chars[i].Offset = pos;

                  // Copy character graphic to the buffer and embolden it

                  int oldwidth = (Chars[i].Width+7)>>3;
                  int newwidth = (Chars[i].Width+8)>>3;
                  for (int y=0; y < Header.pixel_height; y++) {
                     for (int xb=0; xb < oldwidth; xb++) {
                        buffer[pos+xb] |= gfx[xb]|(gfx[xb]>>1);
                        if ((xb < newwidth) and (gfx[xb] & 0x01)) buffer[pos+xb+1] |= 0x80;
                     }

                     pos += newwidth;
                     gfx += oldwidth;
                  }

                  Chars[i].Width++;
                  Chars[i].Advance++;
               }
            }

            FreeResource(mData);
            mData = buffer;
         }
         else Result = log.warning(ERR::AllocMemory);
      }

      if (((StyleFlags & FTF::ITALIC) != FTF::NIL) and (!Header.italic)) {
         log.msg("Converting base font graphics data to italic.");

         int size = 0;
         int extra = Header.pixel_height>>2;

         for (int i=0; i < 256; i++) {
            if (Chars[i].Width) size += Header.pixel_height * ((Chars[i].Width+7+extra)>>3);
         }

         uint8_t *buffer;
         if (AllocMemory(size, MEM::UNTRACKED, &buffer) IS ERR::Okay) {
            int pos = 0;
            for (int i=0; i < 256; i++) {
               if (Chars[i].Width) {
                  uint8_t *gfx = mData + Chars[i].Offset;
                  Chars[i].Offset = pos;

                  int oldwidth = (Chars[i].Width+7)>>3;
                  int newwidth = (Chars[i].Width+7+extra)>>3;
                  int italic = Header.pixel_height;
                  uint8_t *dest = buffer + pos;
                  for (int y=0; y < Header.pixel_height; y++) {
                     int dx = italic>>2;
                     for (int sx=0; sx < Chars[i].Width; sx++) {
                        if (gfx[sx>>3] & (0x80>>(sx & 0x07))) {
                           dest[dx>>3] |= (0x80>>(dx & 0x07));
                        }
                        dx++;
                     }

                     pos  += newwidth;
                     dest += newwidth;
                     gfx  += oldwidth;
                     italic--;
                  }

                  Chars[i].Width += extra;
               }
            }

            FreeResource(mData);
            mData = buffer;
         }
         else Result = log.warning(ERR::AllocMemory);
      }
   }

   uint8_t * get_outline()
   {
      if (mOutline) return mOutline;

      int size = 0;
      for (int16_t i=0; i < 256; i++) {
         if (Chars[i].Width) size += (Header.pixel_height+2) * ((Chars[i].Width+9)>>3);
      }

      uint8_t *buffer;
      if (AllocMemory(size, MEM::UNTRACKED, &buffer) != ERR::Okay) return nullptr;

      int pos = 0;
      for (int16_t i=0; i < 256; i++) {
         if (Chars[i].Width) {
            auto gfx = mData + Chars[i].Offset;
            Chars[i].OutlineOffset = pos;

            int oldwidth = (Chars[i].Width+7)>>3;
            int newwidth = (Chars[i].Width+9)>>3;

            auto dest = buffer + pos;

            dest += newwidth; // Start ahead of line 0
            for (int sy=0; sy < Header.pixel_height; sy++) {
               int dx = 1;
               for (int sx=0; sx < Chars[i].Width; sx++) {
                  if (gfx[sx>>3] & (0x80>>(sx & 0x07))) {
                     if ((sx >= Chars[i].Width-1) or (!(gfx[(sx+1)>>3] & (0x80>>((sx+1) & 0x07))))) dest[(dx+1)>>3] |= (0x80>>((dx+1) & 0x07));
                     if ((sx IS 0) or (!(gfx[(sx-1)>>3] & (0x80>>((sx-1) & 0x07))))) dest[(dx-1)>>3] |= (0x80>>((dx-1) & 0x07));
                     if ((sy < 1) or (!(gfx[(sx>>3)-oldwidth] & (0x80>>(sx & 0x07))))) dest[(dx>>3)-newwidth] |= (0x80>>(dx & 0x07));
                     if ((sy >= Header.pixel_height-1) or (!(gfx[(sx>>3)+oldwidth] & (0x80>>(sx & 0x07))))) dest[(dx>>3)+newwidth] |= (0x80>>(dx & 0x07));
                  }
                  dx++;
               }

               pos  += newwidth;
               dest += newwidth;
               gfx  += oldwidth;
            }
            pos += newwidth * 2;
         }
      }

      mOutline = buffer;
      return mOutline;
   }

   ~BitmapCache() {
      if (OpenCount) {
         pf::Log log(__FUNCTION__);
         log.warning("Removing \"%s : %d : $%.8x\" with an open count of %d", Path.c_str(), Header.nominal_point_size, int(StyleFlags), OpenCount);
      }
      if (mData) { FreeResource(mData); mData = nullptr; }
      if (mOutline) { FreeResource(mOutline); mOutline = nullptr; }
   }
};

static std::list<BitmapCache> glBitmapCache;
static APTR glCacheTimer = nullptr;

//********************************************************************************************************************
// Assumes a cache lock is held on being called.

static BitmapCache * check_bitmap_cache(extFont *Self, FTF Style)
{
   pf::Log log(__FUNCTION__);

   for (auto & cache : glBitmapCache) {
      if (cache.Result != ERR::Okay) continue;

      if (iequals(cache.Path.c_str(), Self->Path)) {
         if (cache.StyleFlags IS Style) {
            if (Self->Point IS cache.Header.nominal_point_size) {
               log.trace("Exists in cache (count %d) %s : %s", cache.OpenCount, cache.Path.c_str(), Self->prvStyle);
               return &cache;
            }
            else log.trace("Failed point check %.2f / %d", Self->Point, cache.Header.nominal_point_size);
         }
         else log.trace("Failed style check $%.8x != $%.8x", Style, cache.StyleFlags);
      }
   }

   return nullptr;
}

//********************************************************************************************************************

ERR bitmap_cache_cleaner(OBJECTPTR Subscriber, int64_t Elapsed, int64_t CurrentTime)
{
   pf::Log log(__FUNCTION__);

   log.msg("Checking bitmap font cache for unused fonts...");

   CACHE_LOCK lock(glCacheMutex);
   for (auto it=glBitmapCache.begin(); it != glBitmapCache.end(); ) {
      if (!it->OpenCount) it = glBitmapCache.erase(it);
      else it++;
   }
   glCacheTimer = nullptr;
   return ERR::Terminate;
}
