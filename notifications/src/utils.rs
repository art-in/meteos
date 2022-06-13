pub fn beautiful_string_join(strings: Vec<&str>) -> String {
    if strings.is_empty() {
        String::new()
    } else if strings.len() == 1 {
        strings[0].into()
    } else if strings.len() == 2 {
        strings.join(" and ")
    } else {
        let all_except_last = &strings[0..strings.len() - 1];
        let mut all_except_last = all_except_last.join(", ");
        all_except_last.push_str(" and ");
        all_except_last.push_str(strings[strings.len() - 1]);
        all_except_last
    }
}

#[cfg(test)]
mod test_beautiful_string_join {
    use super::beautiful_string_join;

    #[test]
    fn no_strings() {
        let res = beautiful_string_join(Vec::new());
        assert_eq!(res, "");
    }

    #[test]
    fn one_string() {
        let res = beautiful_string_join(vec!["foo"]);
        assert_eq!(res, "foo");
    }

    #[test]
    fn two_strings() {
        let res = beautiful_string_join(vec!["foo", "bar"]);
        assert_eq!(res, "foo and bar");
    }

    #[test]
    fn three_strings() {
        let res = beautiful_string_join(vec!["foo", "bar", "baz"]);
        assert_eq!(res, "foo, bar and baz");
    }

    #[test]
    fn dynamic_strings() {
        let strings: Vec<String> = vec!["foo".into(), "bar".into()];
        let res = beautiful_string_join(strings.iter().map(|s| &s[..]).collect());
        assert_eq!(res, "foo and bar");
    }
}
