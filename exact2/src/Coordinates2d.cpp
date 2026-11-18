struct Coordinates2d
{
    int x,y;
    Coordinates2d(int _x, int _y)
    {
        x = _x;
        y = _y;
    }
    Coordinates2d(){}

    void setx(int _x)
    {
        x = _x;
    }
    void sety(int _y)
    {
        y = _y;
    }
    bool operator<(const Coordinates2d& other) const
    {
        if (x != other.x)
            return x < other.x;
        return y < other.y;
    }
};
