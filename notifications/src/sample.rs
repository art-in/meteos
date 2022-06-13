use crate::config::EnvironmentalReadingRanges;
use serde::Deserialize;
use std::ops::Range;

#[derive(Deserialize, Debug, Clone)]
pub struct Sample {
    #[serde(alias = "c")]
    pub co2: f64,
    #[serde(alias = "h")]
    pub humidity: f64,
    #[serde(alias = "p")]
    pub pressure: f64,
    #[serde(alias = "t")]
    pub temperature: f64,
    #[serde(alias = "u")]
    pub time: String,
}

const OPTIMAL_ICON: &str = "✅";
const ABOVE_OPTIMAL_ICON: &str = "⬆";
const BELOW_OPTIMAL_ICON: &str = "⬇";

impl Sample {
    pub fn format_as_markdown(&self, optimal_ranges: &EnvironmentalReadingRanges) -> String {
        format!(
            "```\n\
            {tem_icon} temperature: {tem} °C\n\
            {hum_icon} humidity:    {hum} %\n\
            {pre_icon} pressure:    {pre} mm/hg\n\
            {co2_icon} co2:         {co2} ppm\n\
            ```",
            tem = self.temperature,
            hum = self.humidity,
            pre = self.pressure,
            co2 = self.co2,
            tem_icon =
                Sample::get_reading_status_icon(self.temperature, &optimal_ranges.temperature),
            hum_icon = Sample::get_reading_status_icon(self.humidity, &optimal_ranges.humidity),
            pre_icon = Sample::get_reading_status_icon(self.pressure, &optimal_ranges.pressure),
            co2_icon = Sample::get_reading_status_icon(self.co2, &optimal_ranges.co2),
        )
    }

    fn get_reading_status_icon(reading: f64, optimal_range: &Range<f64>) -> &'static str {
        if reading < optimal_range.start {
            BELOW_OPTIMAL_ICON
        } else if reading > optimal_range.end {
            ABOVE_OPTIMAL_ICON
        } else {
            OPTIMAL_ICON
        }
    }
}
