#include "io.h"

#include <stdio.h>
#include <stdlib.h>
#include "zephyr/atomic.h"

enum {
    /* The rumble motor should be on. */
    IO_RUMBLE_MOTOR_ON,

    /* Flag for detecting when all buttons are released. */
    IO_NO_BTNS_PRESSED,

    /* Flag to force reporting emptied slot. */

    IO_FORCE_EMPTIED,

    /* Flag to control menu rumble feedback. */
    IO_RUMBLE_FEEDBACK,

    /* Is the GC controller a WaveBird? */
    IO_CTRL_PENDING_INIT,

    /* WaveBird association state. */
    IO_CTRL_READY,

    /* Adaptor bypass mode. */
    IO_BYPASS_MODE,

    /* Joystick calibrated. */
    IO_CALIBRATED,

    /* Flag for tracking if remap source is an axis. */
    IO_AXIS,

    /* Flag for button layout modification. */
    IO_LAYOUT_MODIFIER,

    /* Flag for N64 CTRL2. */
    IO_CTRL2,

    /* Set when we want to remap an analog trigger. */
    IO_TRIGGER,

    /* Joystick menu flags. */
    IO_CS,
    IO_AXIS_Y,

    /* 1st controller mute flag. */
    IO_MUTE,

    /* Menu multi-level option flags. */
    IO_MODE,
    IO_LAYOUT,
    IO_JOYSTICK,
    IO_PRESET,
    IO_REMAP,
    IO_SPECIAL,

    /* We're waiting for a button to be released, cleared when no buttons are pressed. */
    IO_WAITING_FOR_RELEASE,

    /* Menu levels flags. */
    IO_MENU_LEVEL1,
    IO_MENU_LEVEL2,
    IO_MENU_LEVEL3,
};

const uint32_t generic_mask[32] =
{
/*  DU              DL              DR              DD               LU             LL              LR              LD               */
    (1U << BTN_DU), (1U << BTN_DL), (1U << BTN_DR), (1U << BTN_DD), (1U << BTN_LU), (1U << BTN_LL), (1U << BTN_LR), (1U << BTN_LD),
/*  BU              BL              BR              BD               RU             RL              RR              RD               */
    (1U << BTN_BU), (1U << BTN_BL), (1U << BTN_BR), (1U << BTN_BD), (1U << BTN_RU), (1U << BTN_RL), (1U << BTN_RR), (1U << BTN_RD),
/*  LA              LM              RA              RM               LS             LG              LJ              RS               */
    (1U << BTN_LA), (1U << BTN_LM), (1U << BTN_RA), (1U << BTN_RM), (1U << BTN_LS), (1U << BTN_LG), (1U << BTN_LJ), (1U << BTN_RS),
/*  RG              RJ              SL              HM               ST             BE                                               */
    (1U << BTN_RG), (1U << BTN_RJ), (1U << BTN_SL), (1U << BTN_HM), (1U << BTN_ST), (1U << BTN_BE), 0, 0
};

const uint8_t generic_axes_idx[6] =
{
    AXIS_LX, AXIS_LY, AXIS_RX, AXIS_RY, TRIG_L, TRIG_R
};

const uint8_t nes_mask[32] =
{
/*  DU    DL    DR    DD    LU    LL    LR    LD    BU    BL    BR    BD    RU    RL    RR    RD    */
    0x08, 0x02, 0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x10, 0x00, 0x00, 0x00
/*  LA    LM    RA    RM    LS    LG    LJ    RS    RG    RJ    SL    HM    ST    BE                */
};

const uint16_t snes_mask[32] =
{
/*  DU      DL      DR      DD      LU      LL      LR      LD      BU      BL      BR      BD      RU      RL      RR      RD      */
    0x0008, 0x0002, 0x0001, 0x0004, 0x0000, 0x0000, 0x0000, 0x0000, 0x4000, 0x0040, 0x8000, 0x0080, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x2000, 0x0000, 0x1000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0020, 0x0000, 0x0010, 0x0000, 0x0000, 0x0000
/*  LA      LM      RA      RM      LS      LG      LJ      RS      RG      RJ      SL      HM      ST      BE                      */
};

const uint16_t n64_mask[32] =
{
/*  DU      DL      DR      DD      LU      LL      LR      LD      BU      BL      BR      BD      RU      RL      RR      RD      */
    0x0008, 0x0002, 0x0001, 0x0004, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0040, 0x0000, 0x0080, 0x0800, 0x0200, 0x0100, 0x0400,
    0x0000, 0x2000, 0x0000, 0x1000, 0x0020, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0010, 0x0000, 0x0000, 0x0000
/*  LA      LM      RA      RM      LS      LG      LJ      RS      RG      RJ      SL      HM      ST      BE                      */
};

const uint32_t wiiu_mask[32] =
{
/*  DU       DL       DR       DD       LU       LL       LR       LD       BU       BL       BR       BD       RU       RL       RR       RD       */
    0x00100, 0x00200, 0x00080, 0x00040, 0x00000, 0x00000, 0x00000, 0x00000, 0x00800, 0x02000, 0x01000, 0x04000, 0x00000, 0x00000, 0x00000, 0x00000,
    0x00000, 0x00020, 0x00000, 0x00002, 0x08000, 0x00000, 0x20000, 0x00400, 0x00000, 0x10000, 0x00010, 0x00008, 0x00004, 0x00000, 0x00000, 0x00000
/*  LA       LM       RA       RM       LS       LG       LJ       RS       RG       RJ       SL       HM       ST       BE                         */
};
const uint8_t wiiu_axes_idx[6] =
{
/*  LX       LY       RX       RY       TL      TR     */
    0,       2,       1,       3,       0,      0
};

static atomic_t io_flags = 0;
static int32_t axis_cal[6] = 0;

static uint8_t map_table[32] =
{
    /*DU*/BTN_DU, /*DL*/BTN_DL, /*DR*/BTN_DR, /*DD*/BTN_DD, /*LU*/BTN_LU, /*LL*/BTN_LL, /*LR*/BTN_LR, /*LD*/BTN_LD,
    /*BU*/BTN_RL, /*BL*/BTN_BL, /*BR*/BTN_RD, /*BD*/BTN_BD, /*RU*/BTN_RU, /*RL*/BTN_RL, /*RR*/BTN_RR, /*RD*/BTN_RD,
    /*LA*/BTN_NN, /*LM*/BTN_LM, /*RA*/BTN_NN, /*RM*/BTN_RM, /*LS*/BTN_LS, /*LG*/BTN_NN, /*LJ*/BTN_NN, /*RS*/BTN_LS,
    /*RG*/BTN_NN, /*RJ*/BTN_NN, /*SL*/BTN_SL, /*HM*/BTN_HM, /*ST*/BTN_ST, /*BE*/BTN_BE, BTN_NN, BTN_NN
};

static inline void set_calibration(int32_t *var, struct axis *axis)
{
    if (axis->sign) {
        *var = axis->ideal - axis->value.sign;
    }
    else {
        *var = axis->ideal - axis->value.unsign;
    }
}

static inline void apply_calibration(int32_t cal, struct axis *axis) {
    /* no clamping, controller really bad if required */
    axis->value.unsign += cal;
}

static inline void apply_deadzone(uint16_t *val) {
    if (*val >= AXIS_DEAD_ZONE) {
        *val -= AXIS_DEAD_ZONE;
    }
    else if (*val <= AXIS_DEAD_ZONE) {
        *val += AXIS_DEAD_ZONE;
    }
    else {
        *val = 0;
    }
}

static void map_to_n64_axis(struct io* output, uint8_t btn_id, int8_t value) {
    switch (btn_id) {
        case BTN_LU:
            if (abs(value) > abs(output->io.n64.ls_y_axis)) {
                output->io.n64.ls_y_axis = value;
            }
            break;
        case BTN_LD:
            if (abs(value) > abs(output->io.n64.ls_y_axis)) {
                output->io.n64.ls_y_axis = -value;
            }
            break;
        case BTN_LL:
            if (abs(value) > abs(output->io.n64.ls_x_axis)) {
                output->io.n64.ls_x_axis = -value;
            }
            break;
        case BTN_LR:
            if (abs(value) > abs(output->io.n64.ls_x_axis)) {
                output->io.n64.ls_x_axis = value;
            }
            break;
    }
};

static void map_axis_to_buttons_axis(struct io* output, uint8_t btn_n, uint8_t btn_p, int8_t value) {
    if (value >= 0x0) {
        if (value > AXIS_BTN_THRS) {
            atomic_set_bit(&io_flags, IO_NO_BTNS_PRESSED);
            output->io.n64.buttons |= n64_mask[map_table[btn_p]];
        }
        map_to_n64_axis(output, map_table[btn_p], value);
    }
    else {
        if (value < -AXIS_BTN_THRS) {
            atomic_set_bit(&io_flags, IO_NO_BTNS_PRESSED);
            output->io.n64.buttons |= n64_mask[map_table[btn_n]];
        }
        map_to_n64_axis(output, map_table[btn_n], -value);
    }
}

static void menu(struct io *input)
{
    if (~input->io.wiiu_pro.buttons & BTN_HM) {
        atomic_set_bit(&io_flags, IO_WAITING_FOR_RELEASE);
        printf("JG2019 In Menu\n");
    }
}

void translate_status(struct io *input, struct io* output) {
    struct generic_map generic;
    uint8_t i;
    int8_t scaled_lx, scaled_ly, scaled_rx, scaled_ry;

    /* Reset N64 status buffer */
    output->buttons = 0x0000000;
    for (i = 0; i < sizeof(geberic.axes)/sizeof(*generic.axes); i++) {
        generic.axes[i].axis.unsign = 0;
    }

    if (!atomic_test_bit(&io_flags, IO_CALIBRATED)) {
        /* Init calib */
        for (i = 0; i < sizeof(generic.axes)/sizeof(*generic.axes); i++) {
            set_calibration(&axis_cal[i], &generic.axes[i]);
        }
        atomic_set_bit(&io_flags, IO_CALIBRATED);
    }

    /* Apply calib */
    for (i = 0; i < sizeof(output->axes)/sizeof(*output->axes); i++) {
        apply_calibration(axis_cal[i], &output->axes[i]);
    }

    /* Apply deadzone */
    apply_deadzone(&input->io.wiiu_pro.ls_x_axis);
    apply_deadzone(&input->io.wiiu_pro.ls_y_axis);
    apply_deadzone(&input->io.wiiu_pro.rs_x_axis);
    apply_deadzone(&input->io.wiiu_pro.rs_y_axis);

    /* Execute menu if Home buttons pressed */
    if (atomic_test_bit(&io_flags, IO_WAITING_FOR_RELEASE)) {
        menu(input);
    }

    /* Scale axis */
    scaled_lx = (input->io.wiiu_pro.ls_x_axis >> 4) - 0x80;
    scaled_ly = (input->io.wiiu_pro.ls_y_axis >> 4) - 0x80;
    scaled_rx = (input->io.wiiu_pro.rs_x_axis >> 4) - 0x80;
    scaled_ry = (input->io.wiiu_pro.rs_y_axis >> 4) - 0x80;

    /* Set responce curve */

    /* Clear flag, will be reset if any buttons is pressed */
    atomic_clear_bit(&io_flags, IO_NO_BTNS_PRESSED);

    /* Map axis to */
    map_axis_to_buttons_axis(output, BTN_LL, BTN_LR, scaled_lx);
    map_axis_to_buttons_axis(output, BTN_LD, BTN_LU, scaled_ly);
    map_axis_to_buttons_axis(output, BTN_RL, BTN_RR, scaled_rx);
    map_axis_to_buttons_axis(output, BTN_RD, BTN_RU, scaled_ry);

    /* Map buttons to */
    for (i = 0; i < 32; i++) {
        if (~input->io.wiiu_pro.buttons & wiiu_mask[i]) {
            atomic_set_bit(&io_flags, IO_NO_BTNS_PRESSED);
            output->io.n64.buttons |= n64_mask[map_table[i]];
            map_to_n64_axis(output, map_table[i], 0x54);
        }
    }
    if (atomic_test_bit(&io_flags, IO_NO_BTNS_PRESSED)) {
        atomic_clear_bit(&io_flags, IO_WAITING_FOR_RELEASE);
    }
}

void wiiu_pro_to_generic(void *map, struct generic_map *generic) {
    struct wiiu_pro_map *specific = (struct wiiu_pro_map *)map;
    uint8_t i;

    memset(generic, 0, sizeof(*generic));
    for (i = 0; i < 30; i++) {
        if (~specific.buttons & wiiu_mask[i]) {
            generic->buttons |= generic_mask[i];
        }
    }
    for (i = 0; i < sizeof(specific->axes)/sizeof(*specific->axes); i++) {
        generic->axes[i].value.unsign = specific->axes[wiiu_axes_idx[i]];
    }
}
