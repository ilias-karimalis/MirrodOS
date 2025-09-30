#include <stdbool.h>
#include <types/error.h>

static struct str_view error_strings[] = {
        [EC_SUCCESS] = SV("EC_SUCCESS: No error."),
};

error_t
error_push(error_t err, enum ErrorCode code)
{
        err <<= sizeof(enum ErrorCode);
        err |= (u64)code;
        return err;
}

error_t
error_pop(error_t err)
{
        return err >> sizeof(enum ErrorCode);
}

enum ErrorCode
error_top(error_t err)
{
        return (enum ErrorCode)(err & 0xFF);
}

bool
error_is_ok(error_t err)
{
        return error_top(err) == EC_SUCCESS;
}

bool
error_is_err(error_t err)
{
        return !error_is_ok(err);
}

struct str_view
error_string(error_t err)
{
        return error_strings[error_top(err)];
}