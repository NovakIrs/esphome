import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import CONF_HAS_TARGET, CONF_ID, DEVICE_CLASS_OCCUPANCY

from .. import CONF_LD2410S_ID, LD2410S, ld2410s_ns

LD2410SCalibrationUpdateBinarySensor = ld2410s_ns.class_(
    "LD2410SCalibrationUpdateBinarySensor", binary_sensor.BinarySensor, cg.Component
)
LD2410SPresenceBinarySensor = ld2410s_ns.class_(
    "LD2410SPresenceBinarySensor", binary_sensor.BinarySensor, cg.Component
)

HAS_CALIBRATION_RUNNING = "has_calibration_running"


CONFIG_SCHEMA = {
    cv.GenerateID(CONF_LD2410S_ID): cv.use_id(LD2410S),
    cv.Optional(HAS_CALIBRATION_RUNNING): binary_sensor.binary_sensor_schema(
        LD2410SCalibrationUpdateBinarySensor, icon="mdi:exclamation"
    ),
    cv.Optional(CONF_HAS_TARGET): binary_sensor.binary_sensor_schema(
        LD2410SPresenceBinarySensor,
        device_class=DEVICE_CLASS_OCCUPANCY,
        icon="mdi:motion-sensor",
    ),
}


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    if CONF_HAS_TARGET in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_HAS_TARGET])
        cg.add(var.set_presence_sensor(sens))
    if HAS_CALIBRATION_RUNNING in config:
        sens = await binary_sensor.new_binary_sensor(config[HAS_CALIBRATION_RUNNING])
        cg.add(var.set_calibration_update_sensor(sens))
    ld2410s = await cg.get_variable(config[CONF_LD2410S_ID])
    cg.add(ld2410s.register_listener(var))
