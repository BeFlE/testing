import esphome.codegen as cg
from esphome.components import i2c, sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_MOISTURE,
    CONF_ID,
    CONF_TEMPERATURE,
    DEVICE_CLASS_MOISTURE,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    ICON_WATER_PERCENT,
    UNIT_CELSIUS,
    UNIT_PERCENT,
)

DEPENDENCIES = ["i2c"]

somose_ns = cg.esphome_ns.namespace("somose")
SOMOSE = somose_ns.class_("SOMOSE", cg.PollingComponent, i2c.I2CDevice)
EnergyMode = somose_ns.enum("EnergyMode_t")
MoistureData = somose_ns.enum("Moisture_Data_t")

ENERGY_MODEs = {
  "CONTINOUS": EnergyMode.continous,
  "ENERGY_SAVING": EnergyMode.energy_saving,
}

MOISTURE_DATAs = {
    "LAST": MoistureData.last,
    "AVERAGE": MoistureData.average,
    "RAW": MoistureData.raw,
}

CONF_ENERGY_MODE = "energy_mode"
CONF_MOISTURE_DATA = "moisture_data"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(SOMOSE),
            cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MOISTURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
                icon=ICON_WATER_PERCENT,
                device_class=DEVICE_CLASS_MOISTURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ENERGY_MODE, default="continous"): cv.enum(
            ENERGY_MODEs, upper=True, space="_"
            ),
            cv.Optional(CONF_MOISTURE_DATA, default="average"): cv.enum(
            MOISTURE_DATAs, upper=True, space="_"
            ),
        }
    )
    .extend(cv.polling_component_schema("20s"))
    .extend(i2c.i2c_device_schema(default_address=0x55))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if temperature_config := config.get(CONF_TEMPERATURE):
        sens = await sensor.new_sensor(temperature_config)
        cg.add(var.set_temperature_sensor(sens))

    if moisture_config := config.get(CONF_MOISTURE):
        sens = await sensor.new_sensor(moisture_config)
        cg.add(var.set_moisture_sensor(sens))

    cg.add(var.set_Energy_Mode(config[CONF_ENERGY_MODE]))

    cg.add(var.set_Moisture_Data(config[CONF_MOISTURE_DATA]))