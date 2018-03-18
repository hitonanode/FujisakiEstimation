#pragma once


namespace stfmest
{
    enum FujisakiemError
    {
        NO_ERROR = 0,
        SIZE_INVALID = 100,
        INPUT_VECTOR_SIZE_MISMATCH,
        TRANSPARAM_VECTOR_SIZE_INVALID,

        VALUE_INVALID = 200,
        CONFIG_VALUE_INVALID,
        TRANSPARAM_PROB_INVALID,

        HMM_SETUP_ERROR = 300,

        CONSISTENCY_ERROR = 400,
        PHASE_DURATION_MISMATCH,

        NOSOLUTION_ERROR = 500,

        NOFILE_ERROR = 600,

        FUTURE_IMPLEMENT = 1000
    };
}
