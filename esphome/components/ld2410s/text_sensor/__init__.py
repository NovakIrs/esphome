import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID, ENTITY_CATEGORY_DIAGNOSTIC, ICON_CHIP

from .. import CONF_LD2410S_ID, LD2410S, ld2410s_ns

LD2410STextSensor = ld2410s_ns.class_(
    "LD2410STextSensor", text_sensor.TextSensor, cg.Component
)

CONF_ENERGY_VALUES = "energy_values"
CONF_FW_VERSION = "fw_version"
CONF_THRESHOLD_TRIGGERS = "threshold_triggers"
CONF_THRESHOLD_HOLDS = "threshold_holds"
CONF_THRESHOLD_SNRS = "threshold_snrs"

CONFIG_SCHEMA = cv.All(
    cv.COMPONENT_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(LD2410STextSensor),
            cv.GenerateID(CONF_LD2410S_ID): cv.use_id(LD2410S),
            cv.Optional(CONF_FW_VERSION): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC, icon=ICON_CHIP
            ),
            cv.Optional(CONF_THRESHOLD_TRIGGERS): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:tune-variant",
            ),
            cv.Optional(CONF_THRESHOLD_HOLDS): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:tune-variant",
            ),
            cv.Optional(CONF_THRESHOLD_SNRS): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:tune-variant",
            ),
            cv.Optional(CONF_ENERGY_VALUES): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:lightning-bolt",
            ),
        }
    ),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    if CONF_FW_VERSION in config:
        sens = await text_sensor.new_text_sensor(config[CONF_FW_VERSION])
        cg.add(var.set_fw_version_text_sensor(sens))

    if CONF_THRESHOLD_TRIGGERS in config:
        sens = await text_sensor.new_text_sensor(config[CONF_THRESHOLD_TRIGGERS])
        cg.add(var.set_threshold_trigger_text_sensor(sens))

    if CONF_THRESHOLD_HOLDS in config:
        sens = await text_sensor.new_text_sensor(config[CONF_THRESHOLD_HOLDS])
        cg.add(var.set_threshold_hold_text_sensor(sens))

    if CONF_THRESHOLD_SNRS in config:
        sens = await text_sensor.new_text_sensor(config[CONF_THRESHOLD_SNRS])
        cg.add(var.set_threshold_snr_text_sensor(sens))

    if CONF_ENERGY_VALUES in config:
        sens = await text_sensor.new_text_sensor(config[CONF_ENERGY_VALUES])
        cg.add(var.set_energy_values_text_sensor(sens))

    ld2410s = await cg.get_variable(config[CONF_LD2410S_ID])
    cg.add(ld2410s.register_listener(var))
