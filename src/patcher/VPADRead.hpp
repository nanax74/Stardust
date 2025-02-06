class VPADRead
{
public:
    static void Start();
    static void Update();
    static bool GetIsStarted() {return isStarted;}
    static void SetIsStarted(bool value) {isStarted = value;}

private:
    static bool isStarted;
};