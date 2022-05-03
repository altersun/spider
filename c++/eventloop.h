

namespace Spider {



class Handle {
    public:
        Handle();
        void Cancel();
        void Cancelled();
        unsigned long int GetID();
        constexpr bool operator==(const Handle& lhs, const Handle& rhs) {return lhs.GetID() == rhs.GetID();}
        

    private:    
        unsigned long int ID;
};



class EventLoop {
    private:
         EventLoop() {}



};






} // end namespace Spider