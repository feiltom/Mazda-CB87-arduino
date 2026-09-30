import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_MAX_VALUE, CONF_MIN_VALUE, CONF_STEP
from esphome.core import CORE

from .. import AircoreGauge, aircore_gauge_ns

CONF_GAUGE_ID = "gauge_id"

AircoreGaugeNumber = aircore_gauge_ns.class_("AircoreGaugeNumber", number.Number, cg.Component)

# min/max par defaut : ceux de la jauge (min_value / max_value)
CONFIG_SCHEMA = (
    number.number_schema(AircoreGaugeNumber)
    .extend(
        {
            cv.Required(CONF_GAUGE_ID): cv.use_id(AircoreGauge),
            cv.Optional(CONF_MIN_VALUE): cv.float_,
            cv.Optional(CONF_MAX_VALUE): cv.float_,
            cv.Optional(CONF_STEP, default=1.0): cv.positive_float,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


def _gauge_config(gauge_id):
    for conf in CORE.config.get("aircore_gauge", []):
        if conf[CONF_ID] == gauge_id:
            return conf
    raise cv.Invalid(f"Jauge {gauge_id} introuvable")


async def to_code(config):
    gauge_conf = _gauge_config(config[CONF_GAUGE_ID])
    lo = min(gauge_conf[CONF_MIN_VALUE], gauge_conf[CONF_MAX_VALUE])
    hi = max(gauge_conf[CONF_MIN_VALUE], gauge_conf[CONF_MAX_VALUE])
    var = await number.new_number(
        config,
        min_value=config.get(CONF_MIN_VALUE, lo),
        max_value=config.get(CONF_MAX_VALUE, hi),
        step=config[CONF_STEP],
    )
    await cg.register_component(var, config)
    await cg.register_parented(var, config[CONF_GAUGE_ID])
