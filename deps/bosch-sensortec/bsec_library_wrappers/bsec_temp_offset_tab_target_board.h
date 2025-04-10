typedef struct {
    float range_min;
    float range_max;
    float w[4];
    float offset_ref;
} temp_offset_cf_param_t;


#define OFFSET_REF_CFG1 2.0f
const 
temp_offset_cf_param_t tocp_cfg1[] = {
    {-45, 15,   {4.68f, 0, 0, 0}, OFFSET_REF_CFG1},
    {15, 30,    {4.57f, 7.56e-3, 0, 0}, OFFSET_REF_CFG1},
    {30, 55,    {4.39f, 0.0151f, 0, 0}, OFFSET_REF_CFG1},
    {55, 65,    {95.3f, -3.26f, 0.0295f, 0}, OFFSET_REF_CFG1},
    {65, 85,    {8.24f, 0, 0, 0}, OFFSET_REF_CFG1},
};

#define OFFSET_REF_CFG2 2.0f
const 
temp_offset_cf_param_t tocp_cfg2[] = {
    {-45, 15,   {1.62f, 0, 0, 0}, OFFSET_REF_CFG2},
    {15, 28,    {-21.3f, 2.84f, -0.108f, 1.37e-3}, OFFSET_REF_CFG2},
    {28, 50,    {2.93f, 0.0243f, 0, 0}, OFFSET_REF_CFG2},
    {50, 62,    {125.8f, -4.86f, 0.0485f, 0}, OFFSET_REF_CFG2},
    {62, 85,    {10.91f,0, 0, 0}, OFFSET_REF_CFG2},
};

#define OFFSET_REF_CFG3 2.0f
const 
temp_offset_cf_param_t tocp_cfg3[] = {
    {-45, 15,   {7.08f, 0, 0, 0}, OFFSET_REF_CFG3},
    {15, 28,    {6.61f, 0.0315f, 0, 0}, OFFSET_REF_CFG3},
    {28, 50,    {6.07f, 0.0451f, 0, 0}, OFFSET_REF_CFG3},
    {50, 62,    {-66.3f, 2.58f, -0.0218f, 0}, OFFSET_REF_CFG3},
    {62, 85,    {9.86f, 0, 0, 0}, OFFSET_REF_CFG3},
};


#define OFFSET_REF_CFG4 2.0f
const 
temp_offset_cf_param_t tocp_cfg4[] = {
    {-45, 15,   {6.94f, 0, 0, 0}, OFFSET_REF_CFG4},
    {15, 22,    {6.51f, 0.0287f, 0, 0}, OFFSET_REF_CFG4},
    {22, 57.5f, {6.21f, 0.0396f, 0, 0}, OFFSET_REF_CFG4},
    {57.5f, 65, {-120.5f, 4.03f, -0.0311f, 0}, OFFSET_REF_CFG4},
    {65, 85,    {10.05f, 0, 0, 0}, OFFSET_REF_CFG4},
};
