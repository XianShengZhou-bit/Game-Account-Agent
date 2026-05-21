import logging
import os
from pathlib import Path
from dotenv import load_dotenv

load_dotenv()

DEBUG = os.getenv("DEBUG", "false").lower() == "true"

# 获取 STATIC_DIR，若未设置则使用项目根目录下的 static 目录
static_dir_env = os.getenv("STATIC_DIR")
if static_dir_env:
    STATIC_DIR = Path(static_dir_env)
else:
    # 默认使用项目根目录下的 static 目录
    current_file = Path(__file__).resolve()
    project_root = current_file.parent.parent.parent
    STATIC_DIR = project_root / "static"

LOG_FILE = STATIC_DIR / "logs.log"


def _ensure_log_dir() -> None:
    if not STATIC_DIR.exists():
        STATIC_DIR.mkdir(parents=True, exist_ok=True)


def _create_logger(name: str) -> logging.Logger:
    logger = logging.getLogger(name)
    if logger.handlers:
        return logger

    logger.setLevel(logging.DEBUG if DEBUG else logging.INFO)

    if DEBUG:
        handler = logging.StreamHandler()
        handler.setFormatter(logging.Formatter(
            "%(asctime)s - %(name)s - %(levelname)s - [%(filename)s:%(lineno)d] - %(message)s"
        ))
    else:
        _ensure_log_dir()
        handler = logging.FileHandler(LOG_FILE, encoding="utf-8")
        handler.setFormatter(logging.Formatter(
            "%(asctime)s - %(name)s - %(levelname)s - [%(filename)s:%(lineno)d] - %(message)s"
        ))

    logger.addHandler(handler)
    return logger


app_logger = _create_logger("game_account_exchange")


def setup_logger(name: str = "game_account_exchange") -> logging.Logger:
    return _create_logger(name)