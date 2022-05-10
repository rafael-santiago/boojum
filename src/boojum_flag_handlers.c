// INFO(Rafael): With C11 we have _Atomic() facilities.
#if !defined(BOOJUM_WITH_C11)
# include <boojum_proc.h>

int boojum_get_flag(const int *flag, boojum_mutex *mtx) {
    int value = 0;

    if (flag == NULL || mtx == NULL) {
        return 0;
    }

    if (boojum_mutex_lock(mtx) == EXIT_SUCCESS) {
        value = *flag;
        boojum_mutex_unlock(mtx);
    }

    return value;
}

int boojum_set_flag(int *flag, const int value, boojum_mutex *mtx) {
    int err = EXIT_FAILURE;

    if (flag == NULL || mtx == NULL) {
        return EXIT_FAILURE;
    }

    if (boojum_mutex_lock(mtx) == EXIT_SUCCESS) {
        *flag = value;
        boojum_mutex_unlock(mtx);
        err = EXIT_SUCCESS;
    }

    return err;
}
#endif // !defined(BOOJUM_WITH_C11)
