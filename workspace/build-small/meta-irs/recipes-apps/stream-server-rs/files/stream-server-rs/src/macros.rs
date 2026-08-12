
#[macro_export]
macro_rules! blocking {
    ($err_msg:expr, $func:expr) => {{
        let res = tokio::task::spawn_blocking($func).await;
        match res {
            Ok(config) => Ok(BackendResponse::Config(config?)),
            Err(err) => {
                error!("{}: {:?}", $err_msg, err);
                Err(BackendError::RuntimeError)
            }
        }
    }};
}
