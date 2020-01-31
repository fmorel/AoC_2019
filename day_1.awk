BEGIN {
    s=0
}
{
    fuel = int($0/3) - 2;
    while (fuel > 0) {
        s += fuel
        fuel = int(fuel/3) - 2
    }
}
END {
    print s
}
