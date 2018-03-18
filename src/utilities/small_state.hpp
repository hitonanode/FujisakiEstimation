#pragma once

#include <string>
#include <vector>


namespace stfmest
{
    struct SmallState
    {
        int attribute;
        int bigstateId;
        std::vector<unsigned> forwardConnects;
        std::vector<unsigned> backwardConnects;
        bool isStarting;
        bool isEnding;
        SmallState(int attribute, int bigstateId): attribute(attribute), bigstateId(bigstateId)
        {
            forwardConnects.clear();
            backwardConnects.clear();
            isStarting = false;
            isEnding = false;
        }
        SmallState() {}
        inline std::string getStatus()
        {
            std::string retval;
            if (isStarting) retval += "[START]";
            if (isEnding) retval += "[END]";
            retval += "Attribute=" + std::to_string(attribute);
            retval += ", Bigstate_id=" + std::to_string(bigstateId);
            retval += ", Forward:";
            for (unsigned f : forwardConnects) retval += " " + std::to_string(f);
            retval += " Backward:";
            for (unsigned b : backwardConnects) retval += " " + std::to_string(b);
            return retval;
        }

    };
}
