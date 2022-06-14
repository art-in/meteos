use crate::{
    config::ReadingRanges,
    reading::{get_reading_optimality, Optimality},
};
use serde::Deserialize;
use std::ops::Range;

#[derive(Deserialize, Debug, Clone)]
pub struct Sample {
    #[serde(alias = "t")]
    pub temperature: f64,

    #[serde(alias = "h")]
    pub humidity: f64,

    #[serde(alias = "p")]
    pub pressure: f64,

    #[serde(alias = "c")]
    pub co2: f64,

    #[serde(alias = "u")]
    pub time: String,
}

impl Sample {
    pub fn format_as_markdown(&self, optimal_ranges: &ReadingRanges) -> String {
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
            tem_icon = get_reading_optimality_icon(self.temperature, &optimal_ranges.temperature),
            hum_icon = get_reading_optimality_icon(self.humidity, &optimal_ranges.humidity),
            pre_icon = get_reading_optimality_icon(self.pressure, &optimal_ranges.pressure),
            co2_icon = get_reading_optimality_icon(self.co2, &optimal_ranges.co2),
        )
    }
}

fn get_reading_optimality_icon(reading: f64, optimal_range: &Range<f64>) -> &'static str {
    let optimality = get_reading_optimality(reading, optimal_range);

    match optimality {
        Optimality::Optimal => "✅",
        Optimality::AboveOptimal => "⬆",
        Optimality::BelowOptimal => "⬇",
        Optimality::Mixed => unreachable!("single reading cannot have mixed optimality"),
    }
}
