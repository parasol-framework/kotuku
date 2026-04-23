
#include <mutex>

#define FT_DOWNSIZE  6
#define FIXED_DPI 96 // FreeType measurements are based on this DPI.

struct FontCharacter {
   int16_t  Width;
   int16_t  Advance;
   uint16_t Offset;
   uint16_t OutlineOffset;
};

class font_cache { // Represents a font face.  Stored in glCache
public:
   std::string Path; // Path to the font source
   FT_Face Face;     // Truetype font face
   int    Usage;    // Counter for usage of the typeface

   font_cache(std::string pPath, FT_Face pFace) : Path(pPath), Face(pFace), Usage(0) { }

   ~font_cache() {
      pf::Log log;
      pf::SwitchContext ctx(modFont);
      FT_Done_Face(Face);
      log.trace("Terminated cache entry for '%s'", Path.c_str());
   }
};

typedef const std::lock_guard<std::recursive_mutex> CACHE_LOCK;
static std::recursive_mutex glCacheMutex; // Protects access to glCache for multi-threading support
