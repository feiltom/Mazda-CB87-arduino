from esphome import automation
import esphome.codegen as cg
from esphome.components import output, sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_MAX_POWER, CONF_SENSOR, CONF_SPEED, CONF_VALUE

CODEOWNERS = ["@feiltom"]
MULTI_CONF = True

aircore_gauge_ns = cg.esphome_ns.namespace("aircore_gauge")
AircoreGauge = aircore_gauge_ns.class_("AircoreGauge", cg.Component)
SetValueAction = aircore_gauge_ns.class_("SetValueAction", automation.Action)
SetAngleAction = aircore_gauge_ns.class_("SetAngleAction", automation.Action)

CONF_COS_POS = "cos_pos"
CONF_COS_NEG = "cos_neg"
CONF_SIN_POS = "sin_pos"
CONF_SIN_NEG = "sin_neg"
CONF_COS = "cos"
CONF_SIN = "sin"
CONF_COMMON = "common"
CONF_ZERO_OFFSET = "zero_offset"
CONF_MIN_VALUE = "min_value"
CONF_MAX_VALUE = "max_value"
CONF_MIN_ANGLE = "min_angle"
CONF_MAX_ANGLE = "max_angle"
CONF_ANGLE = "angle"

FOUR_WIRE = (CONF_COS_POS, CONF_COS_NEG, CONF_SIN_POS, CONF_SIN_NEG)
THREE_WIRE = (CONF_COS, CONF_SIN, CONF_COMMON)


def _validate_wiring(config):
    four = [k for k in FOUR_WIRE if k in config]
    three = [k for k in THREE_WIRE if k in config]
    if four and three:
        raise cv.Invalid(
            "Choisir soit le cablage 4 fils (cos_pos/cos_neg/sin_pos/sin_neg), "
            "soit le cablage 3 fils (cos/sin/common), pas les deux"
        )
    if four and len(four) != len(FOUR_WIRE):
        raise cv.Invalid(f"Cablage 4 fils incomplet : il faut {', '.join(FOUR_WIRE)}")
    if three and len(three) != len(THREE_WIRE):
        raise cv.Invalid(f"Cablage 3 fils incomplet : il faut {', '.join(THREE_WIRE)}")
    if not four and not three:
        raise cv.Invalid(
            "Aucune sortie : definir cos_pos/cos_neg/sin_pos/sin_neg (4 fils) "
            "ou cos/sin/common (3 fils)"
        )
    if config[CONF_MIN_VALUE] == config[CONF_MAX_VALUE]:
        raise cv.Invalid("min_value et max_value doivent etre differents")
    return config


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AircoreGauge),
            **{cv.Optional(k): cv.use_id(output.FloatOutput) for k in FOUR_WIRE + THREE_WIRE},
            cv.Optional(CONF_ZERO_OFFSET, default=0.0): cv.float_,
            cv.Optional(CONF_MAX_POWER, default="80%"): cv.percentage,
            cv.Optional(CONF_SPEED, default=180.0): cv.positive_float,
            cv.Optional(CONF_MIN_VALUE, default=0.0): cv.float_,
            cv.Optional(CONF_MAX_VALUE, default=100.0): cv.float_,
            cv.Optional(CONF_MIN_ANGLE, default=0.0): cv.float_,
            cv.Optional(CONF_MAX_ANGLE, default=270.0): cv.float_,
            cv.Optional(CONF_SENSOR): cv.use_id(sensor.Sensor),
        }
    ).extend(cv.COMPONENT_SCHEMA),
    _validate_wiring,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    if CONF_COS_POS in config:
        cg.add(
            var.set_four_wire(
                await cg.get_variable(config[CONF_COS_POS]),
                await cg.get_variable(config[CONF_COS_NEG]),
                await cg.get_variable(config[CONF_SIN_POS]),
                await cg.get_variable(config[CONF_SIN_NEG]),
            )
        )
    else:
        cg.add(
            var.set_three_wire(
                await cg.get_variable(config[CONF_COS]),
                await cg.get_variable(config[CONF_SIN]),
                await cg.get_variable(config[CONF_COMMON]),
            )
        )

    cg.add(var.set_zero_offset(config[CONF_ZERO_OFFSET]))
    cg.add(var.set_max_power(config[CONF_MAX_POWER]))
    cg.add(var.set_speed(config[CONF_SPEED]))
    cg.add(var.set_value_range(config[CONF_MIN_VALUE], config[CONF_MAX_VALUE]))
    cg.add(var.set_angle_range(config[CONF_MIN_ANGLE], config[CONF_MAX_ANGLE]))

    if CONF_SENSOR in config:
        cg.add(var.set_sensor(await cg.get_variable(config[CONF_SENSOR])))


@automation.register_action(
    "aircore_gauge.set_value",
    SetValueAction,
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.use_id(AircoreGauge),
            cv.Required(CONF_VALUE): cv.templatable(cv.float_),
        }
    ),
    synchronous=True,
)
async def set_value_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    cg.add(var.set_value(await cg.templatable(config[CONF_VALUE], args, cg.float_)))
    return var


@automation.register_action(
    "aircore_gauge.set_angle",
    SetAngleAction,
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.use_id(AircoreGauge),
            cv.Required(CONF_ANGLE): cv.templatable(cv.float_),
        }
    ),
    synchronous=True,
)
async def set_angle_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    cg.add(var.set_angle(await cg.templatable(config[CONF_ANGLE], args, cg.float_)))
    return var
