import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv
from esphome.const import (
    CONF_CALIBRATION,
    CONF_FACTORY_RESET,
    DEVICE_CLASS_RESTART,
    DEVICE_CLASS_UPDATE,
    ENTITY_CATEGORY_CONFIG,
)

from .. import CONF_LD2410S_ID, LD2410S, ld2410s_ns

LD2410SReadAllButton = ld2410s_ns.class_("LD2410SReadAllButton", button.Button)
LD2410SApplyConfigButton = ld2410s_ns.class_("LD2410SApplyConfigButton", button.Button)
LD2410SCalibrationButton = ld2410s_ns.class_("LD2410SCalibrationButton", button.Button)
LD2410SFactoryResetButton = ld2410s_ns.class_(
    "LD2410SFactoryResetButton", button.Button
)
LD2410SMinimalOutputButton = ld2410s_ns.class_(
    "LD2410SMinimalOutputButton", button.Button
)

CONF_MINIMAL_OUTPUT = "minimal_output"
CONF_READ_ALL = "read_all"
CONF_WRITE_ALL = "write_all"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_LD2410S_ID): cv.use_id(LD2410S),
    cv.Optional(CONF_READ_ALL): button.button_schema(
        LD2410SReadAllButton,
        device_class=DEVICE_CLASS_UPDATE,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon="mdi:reload",
    ),
    cv.Optional(CONF_WRITE_ALL): button.button_schema(
        LD2410SApplyConfigButton,
        device_class=DEVICE_CLASS_RESTART,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon="mdi:content-save",
    ),
    cv.Optional(CONF_CALIBRATION): button.button_schema(
        LD2410SCalibrationButton,
        device_class=DEVICE_CLASS_UPDATE,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon="mdi:refresh-auto",
    ),
    cv.Optional(CONF_FACTORY_RESET): button.button_schema(
        LD2410SFactoryResetButton,
        device_class=DEVICE_CLASS_UPDATE,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon="mdi:factory",
    ),
    cv.Optional(CONF_MINIMAL_OUTPUT): button.button_schema(
        LD2410SMinimalOutputButton,
        device_class=DEVICE_CLASS_UPDATE,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon="mdi:arrow-collapse-horizontal",
    ),
}


async def to_code(config):
    ld2410s_component = await cg.get_variable(config[CONF_LD2410S_ID])

    if read_all := config.get(CONF_READ_ALL):
        b = await button.new_button(read_all)
        await cg.register_parented(b, config[CONF_LD2410S_ID])
        cg.add(ld2410s_component.set_read_all_button(b))

    if write_all := config.get(CONF_WRITE_ALL):
        b = await button.new_button(write_all)
        await cg.register_parented(b, config[CONF_LD2410S_ID])
        cg.add(ld2410s_component.set_write_all_button(b))

    if calibration := config.get(CONF_CALIBRATION):
        b = await button.new_button(calibration)
        await cg.register_parented(b, config[CONF_LD2410S_ID])
        cg.add(ld2410s_component.set_calibration_button(b))

    if factory_reset := config.get(CONF_FACTORY_RESET):
        b = await button.new_button(factory_reset)
        await cg.register_parented(b, config[CONF_LD2410S_ID])
        cg.add(ld2410s_component.set_factory_reset_button(b))

    if toggle_minimal_output := config.get(CONF_MINIMAL_OUTPUT):
        b = await button.new_button(toggle_minimal_output)
        await cg.register_parented(b, config[CONF_LD2410S_ID])
        cg.add(ld2410s_component.set_toggle_minimal_output_button(b))
