import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_SIGNAL_STRENGTH,
    ENTITY_CATEGORY_CONFIG,
    UNIT_DECIBEL,
)

from .. import CONF_LD2410S_ID, LD2410S, ld2410s_ns

LD2410SThresholdTriggerNumber = ld2410s_ns.class_(
    "LD2410SThresholdTriggerNumber", number.Number
)
LD2410SThresholdHoldNumber = ld2410s_ns.class_(
    "LD2410SThresholdHoldNumber", number.Number
)
LD2410SThresholdSnrNumber = ld2410s_ns.class_(
    "LD2410SThresholdSnrNumber", number.Number
)
LD2410SThresholdSelectedGateNumber = ld2410s_ns.class_(
    "LD2410SThresholdSelectedGateNumber", number.Number
)

CONF_THRESHOLD_TRIGGER = "threshold_trigger"
CONF_THRESHOLD_HOLD = "threshold_hold"
CONF_THRESHOLD_SNR = "threshold_snr"
CONF_THRESHOLD_SELECTED_GATE = "threshold_selected_gate"


CONFIG_SCHEMA = {
    cv.GenerateID(CONF_LD2410S_ID): cv.use_id(LD2410S),
    cv.Optional(CONF_THRESHOLD_TRIGGER): number.number_schema(
        LD2410SThresholdTriggerNumber,
        device_class=DEVICE_CLASS_SIGNAL_STRENGTH,
        entity_category=ENTITY_CATEGORY_CONFIG,
        unit_of_measurement=UNIT_DECIBEL,
        icon="mdi:pencil",
    ),
    cv.Optional(CONF_THRESHOLD_HOLD): number.number_schema(
        LD2410SThresholdHoldNumber,
        device_class=DEVICE_CLASS_SIGNAL_STRENGTH,
        entity_category=ENTITY_CATEGORY_CONFIG,
        unit_of_measurement=UNIT_DECIBEL,
        icon="mdi:pencil",
    ),
    cv.Optional(CONF_THRESHOLD_SNR): number.number_schema(
        LD2410SThresholdSnrNumber,
        device_class=DEVICE_CLASS_SIGNAL_STRENGTH,
        entity_category=ENTITY_CATEGORY_CONFIG,
        unit_of_measurement=UNIT_DECIBEL,
        icon="mdi:pencil",
    ),
    cv.Optional(CONF_THRESHOLD_SELECTED_GATE): number.number_schema(
        LD2410SThresholdSelectedGateNumber,
        device_class=DEVICE_CLASS_SIGNAL_STRENGTH,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon="mdi:tune-variant",
    ),
}


async def to_code(config):
    LD2410S_component = await cg.get_variable(config[CONF_LD2410S_ID])
    if threshold_trigger_config := config.get(CONF_THRESHOLD_TRIGGER):
        n = await number.new_number(
            threshold_trigger_config, min_value=10, max_value=95, step=1
        )
        await cg.register_parented(n, config[CONF_LD2410S_ID])
        cg.add(LD2410S_component.set_threshold_trigger_number(n))
    if threshold_hold_config := config.get(CONF_THRESHOLD_HOLD):
        n = await number.new_number(
            threshold_hold_config, min_value=10, max_value=95, step=1
        )
        await cg.register_parented(n, config[CONF_LD2410S_ID])
        cg.add(LD2410S_component.set_threshold_hold_number(n))
    if threshold_snr_config := config.get(CONF_THRESHOLD_SNR):
        n = await number.new_number(
            threshold_snr_config, min_value=5, max_value=63, step=1
        )
        await cg.register_parented(n, config[CONF_LD2410S_ID])
        cg.add(LD2410S_component.set_threshold_snr_number(n))
    if threshold_selected_gate_config := config.get(CONF_THRESHOLD_SELECTED_GATE):
        n = await number.new_number(
            threshold_selected_gate_config, min_value=0, max_value=15, step=1
        )
        await cg.register_parented(n, config[CONF_LD2410S_ID])
        cg.add(LD2410S_component.set_threshold_selected_gate_number(n))
