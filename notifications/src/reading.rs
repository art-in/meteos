use std::ops::Range;

#[derive(PartialEq, Debug, Clone, Copy)]
pub enum Reading {
    Temperature,
    Humidity,
    Pressure,
    Co2,
}

#[derive(PartialEq, Clone, Copy, Debug)]
pub enum Optimality {
    Optimal,
    AboveOptimal,
    BelowOptimal,
    Mixed,
}

#[derive(PartialEq, Clone, Copy, Debug)]
pub struct ReadingOptimality {
    pub reading: Reading,
    pub optimality: Optimality,
}

impl ReadingOptimality {
    pub fn format_as_status_string(&self) -> &'static str {
        match self.reading {
            Reading::Temperature => match self.optimality {
                Optimality::AboveOptimal => "too hot 🥵",
                Optimality::BelowOptimal => "too cold 🥶",
                Optimality::Optimal => "optimal temperature",
                Optimality::Mixed => "fast changing temperature",
            },
            Reading::Humidity => match self.optimality {
                Optimality::AboveOptimal => "too wet 💦",
                Optimality::BelowOptimal => "too dry 🌵",
                Optimality::Optimal => "optimal humidity",
                Optimality::Mixed => "fast changing humidity",
            },
            Reading::Pressure => match self.optimality {
                Optimality::AboveOptimal => "high pressure",
                Optimality::BelowOptimal => "low pressure",
                Optimality::Optimal => "optimal pressure",
                Optimality::Mixed => "fast changing pressure",
            },
            Reading::Co2 => match self.optimality {
                Optimality::AboveOptimal => "high co2 🤢",
                Optimality::BelowOptimal => "low co2 🤥",
                Optimality::Optimal => "optimal co2",
                Optimality::Mixed => "fast changing co2",
            },
        }
    }
}

pub fn get_reading_optimality(reading: f64, optimal_range: &Range<f64>) -> Optimality {
    if reading < optimal_range.start {
        Optimality::BelowOptimal
    } else if reading > optimal_range.end {
        Optimality::AboveOptimal
    } else {
        Optimality::Optimal
    }
}

pub fn get_all_readings_optimality(readings: Vec<f64>, optimal_range: &Range<f64>) -> Optimality {
    let mut is_all_optimal = true;
    let mut is_all_above_optimal = true;
    let mut is_all_below_optimal = true;

    readings.iter().copied().for_each(|r| {
        if r < optimal_range.start {
            is_all_optimal = false;
            is_all_above_optimal = false;
        } else {
            is_all_below_optimal = false;
        }

        if r > optimal_range.end {
            is_all_optimal = false;
            is_all_below_optimal = false;
        } else {
            is_all_above_optimal = false;
        }
    });

    if !is_all_optimal && !is_all_above_optimal && !is_all_below_optimal {
        Optimality::Mixed
    } else if is_all_above_optimal {
        Optimality::AboveOptimal
    } else if is_all_below_optimal {
        Optimality::BelowOptimal
    } else {
        Optimality::Optimal
    }
}
