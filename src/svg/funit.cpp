
svgState::FUNIT::FUNIT(svgState *pState, std::string_view pValue, DU pType, double pMin) noexcept
{
   state = pState;
   auto str = pValue;
   ltrim(str);
   if (str.starts_with('+')) str.remove_prefix(1);
   auto [ end, error ] = std::from_chars(str.data(), str.data() + str.size(), value);
   if (error != std::errc()) { value = 0; return; }

   str = str.substr(end - str.data());
   if (str.starts_with("%")) { value *= 0.01; type = pType != DU::NIL ? pType : DU::SCALED; }
   else {
      type = pType != DU::NIL ? pType : DU::PIXEL;

      if (!str.empty()) {
         if (str.starts_with("em")) value *= state->m_font_size_px; // Multiply the current font's pixel height by the provided em value
         else if (str.starts_with("ex")) value *= state->m_font_size_px * 0.5; // As for em, but multiple by the pixel height of the 'x' character.  If no x character, revert to 0.5em
         else if (str.starts_with("in")) value *= glDisplayDPI; // Inches -> Pixels
         else if (str.starts_with("cm")) value *= (1.0 / 2.54) * glDisplayDPI; // Centimetres -> Pixels
         else if (str.starts_with("mm")) value *= (1.0 / 25.4) * glDisplayDPI; // Millimetres -> Pixels
         else if (str.starts_with("pt")) value *= (4.0 / 3.0); // Points -> Pixels.  A point is 4/3 of a pixel
         else if (str.starts_with("pc")) value *= (4.0 / 3.0) * 12.0; // Pica -> Pixels.  1 Pica is equal to 12 Points
      }

      if (value < pMin) value = pMin;
   }
}
