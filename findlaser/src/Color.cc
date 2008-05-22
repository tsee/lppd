namespace FindLaser {
 // I know, public... but this is just a struct!
  class Color {
    public:
    Color(unsigned char r, unsigned char g, unsigned char b);

    ~Color() {}

    Color GetNormalized();

    unsigned char red;
    unsigned char green;
    unsigned char blue;
  }; // end class Color
} // end namespace FindLaser

