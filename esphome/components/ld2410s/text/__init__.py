import esphome.codegen as cg
from esphome.components import text
import esphome.config_validation as cv
from esphome.const import (
    CONF_ENTITY_CATEGORY,
    CONF_ICON,
    CONF_MODE,
    CONF_SOURCE_ID,
    ENTITY_CATEGORY_CONFIG,
    ICON_MOTION_SENSOR,
    ICON_TIMELAPSE,
    ICON_PULSE,
    CONF_ID
)
from esphome.core.entity_helpers import inherit_property_from

from .. import CONF_LD2410S_ID, LD2410S, ld2410s_ns

LD2410SThresholdTriggerText = ld2410s_ns.class_("LD2410SThresholdTriggerText", text.Text)

CONF_THRESHOLDS_TRIGGER_TEXT = "thresholds_trigger"
CONF_THRESHOLDS_HOLD_TEXT = "thrEsholds_hold"
CONF_THRESHOLDS_SNR_TEXT = "thrEsholds_snr"


CONFIG_SCHEMA = text.TEXT_SCHEMA.extend({
#    cv.GenerateID(): cv.declare_id(LD2410SThresholdTriggerText),
    # cv.GenerateID(CONF_LD2410S_ID): cv.use_id(LD2410S),
    cv.Required(CONF_THRESHOLDS_TRIGGER_TEXT): cv.declare_id(LD2410SThresholdTriggerText),    
}).extend(cv.COMPONENT_SCHEMA)

# CONFIG_SCHEMA = text.TEXT_SCHEMA.extend(
#     {
#         cv.GenerateID(CONF_LD2410S_ID): cv.use_id(LD2410S),
# #        cv.GenerateID(): cv.declare_id(LD2410SThresholdTriggerText),
#         cv.Required(CONF_THRESHOLDS_TRIGGER_TEXT): cv.use_id(LD2410SThresholdTriggerText),
#         cv.Optional(
#             CONF_ENTITY_CATEGORY, default=ENTITY_CATEGORY_CONFIG
#         ): cv.entity_category
#         }).extend(cv.COMPONENT_SCHEMA)

# FINAL_VALIDATE_SCHEMA = cv.All(
#     inherit_property_from(CONF_ICON, CONF_SOURCE_ID),
#     inherit_property_from(CONF_ENTITY_CATEGORY, CONF_SOURCE_ID),
#     inherit_property_from(CONF_MODE, CONF_SOURCE_ID),
# )

# async def to_code(config):
# #    var = cg.new_Pvariable(config[CONF_ID])
#         # sens = await sensor.new_sensor(config[CONF_THRESHOLDS_TRIGGER_TEXT])
#         # cg.add(var.set_distance_sensor(sens))
           
# #    await cg.register_component(var, config)
    
#     #LD2410S_component = await cg.get_variable(config[CONF_LD2410S_ID])
#     #LD2410S_component = await cg.get_variable(config[CONF_THRESHOLDS_TRIGGER_TEXT])
#     if CONF_THRESHOLDS_TRIGGER_TEXT in config:
#         var = await text.new_text(config)
#         await text.register_text(var, config)


#     #LD2410S_component = await cg.get_variable(config[CONF_LD2410S_ID])
#     #    await cg.register_component(var, config)
#     #    await cg.register_parented(var, CONF_LD2410S_ID)

#         # source = await cg.get_variable(config[CONF_THRESHOLDS_TRIGGER_TEXT])
#         # cg.add(var.set_source(source))

#         # cg.add(LD2410S_component.set_max_distance_number(n))
#     # var = cg.new_Pvariable(config[CONF_LD2410S_ID])
#     # await cg.register_component(var, config)
#     # await text.register_text(var, config)


# # async def to_code(config):
# #     LD2410S_component = await cg.get_variable(config[CONF_LD2410S_ID])
# #     if thresholds_trigger := config.get(CONF_THRESHOLDS_TRIGGER_TEXT):
# #         n = await text.new_text(thresholds_trigger)
# #         await cg.register_parented(n, config[CONF_LD2410S_ID])
# # #        cg.add(LD2410S_component.set_thresholds_trigger(n))

#     # var = await text.new_text(config)
#     # await cg.register_component(var, config)

#     # source = await cg.get_variable(config[CONF_THRESHOLDS_TRIGGER_TEXT])
#     # cg.add(var.set_source(source))


# # async def to_code(config):
# #     var = await text.new_text(config)
# #     await cg.register_component(var, config)

# #     source = await cg.get_variable(config[CONF_THRESHOLDS_TRIGGER_TEXT])
# #     cg.add(var.set_source(source))
# # async def to_code(config):
# #     LD2410S_component = await cg.get_variable(config[CONF_LD2410S_ID])
# #     if max_distance_config := config.get(CONF_MAX_DISTANCE):
# #         n = await number.new_number(max_distance_config, min_value=0, max_value=16, step=1)
# #         await cg.register_parented(n, config[CONF_LD2410S_ID])
# #         cg.add(LD2410S_component.set_max_distance_number(n))