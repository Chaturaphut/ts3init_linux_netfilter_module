#ifndef _TS3INIT_MATCH_H
#define _TS3INIT_MATCH_H

/* Enums for get_cookie and get_puzzle matches */
enum
{
    CHK_COMMON_CLIENT_VERSION = 1 << 0,
    CHK_COMMON_VALID_MASK     = (1 << 1) -1,

    CLIENT_VERSION_OFFSET     = 1356998400
};

/* Enums and structs for get_cookie */
enum
{ 
    CHK_GET_COOKIE_CHECK_TIMESTAMP = 1 << 0,
    CHK_GET_COOKIE_VALID_MASK      = (1 << 1) -1
};

struct xt_ts3init_get_cookie_mtinfo
{
    __u8 common_options;
    __u8 specific_options;
    __u16 reserved1;
    __u32 min_client_version;
    __u32 max_utc_offset;
};


/* Enums and structs for get_puzzle */
enum
{
    CHK_GET_PUZZLE_CHECK_COOKIE              = 1 << 0,
    CHK_GET_PUZZLE_RANDOM_SEED_FROM_ARGUMENT = 1 << 1,
    CHK_GET_PUZZLE_RANDOM_SEED_FROM_FILE     = 1 << 2,
    CHK_GET_PUZZLE_VALID_MASK               = (1 << 3) - 1,
};

struct xt_ts3init_get_puzzle_mtinfo
{
    __u8 common_options;
    __u8 specific_options;
    __u16 reserved1;
    __u32 min_client_version;
    __u8 random_seed[RANDOM_SEED_LEN];
    char random_seed_path[RANDOM_SEED_PATH_MAX];
};

/* Enums and structs for generic ts3init */
enum
{
    CHK_TS3INIT_CLIENT              = 1 << 0,
    CHK_TS3INIT_SERVER              = 1 << 1,
    CHK_TS3INIT_COMMAND             = 1 << 2,
    CHK_TS3INIT_VALID_MASK         = (1 << 3) - 1,
};

struct xt_ts3init_mtinfo
{
    __u8 common_options;
    __u8 specific_options;
    __u16 reserved1;
	__u8 command;
};
#endif /* _TS3INIT_MATCH_H */
