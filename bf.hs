{-# LANGUAGE LambdaCase #-}
import Control.Monad (void)
import Data.Bifunctor (second)
import Data.Char (chr, ord)
import Data.Word (Word8)
import System.Environment (getArgs)
import System.Exit (exitFailure, exitSuccess)
import System.IO (BufferMode(..), hPutStrLn, hSetBinaryMode,
                  hSetBuffering, stderr, stdin, stdout)
import System.IO.Error (catchIOError, isEOFError)

name :: String
name = "bf"

abort :: String -> IO a
abort message =
  hPutStrLn stderr (name ++ ": " ++ message) >> exitFailure

data BF = Incr | Decr | Next | Prev | Putc | Getc | While [BF]

parse :: String -> IO [BF]
parse source = parse' source 0 >>= \case
  (_, Nothing, codes) -> return codes
  (_, Just i, _)      -> abort $ "unmatched ] at byte " ++ show i
  where
    parse' :: String -> Int -> IO (String, Maybe Int, [BF])
    parse' ""       i = return ("", Nothing, [])
    parse' ('+':bs) i = second (Incr:) <$> parse' bs (i + 1)
    parse' ('-':bs) i = second (Decr:) <$> parse' bs (i + 1)
    parse' ('>':bs) i = second (Next:) <$> parse' bs (i + 1)
    parse' ('<':bs) i = second (Prev:) <$> parse' bs (i + 1)
    parse' ('.':bs) i = second (Putc:) <$> parse' bs (i + 1)
    parse' (',':bs) i = second (Getc:) <$> parse' bs (i + 1)
    parse' ('[':bs) i = parse' bs (i + 1) >>= \case
      (bs, Just i, cs) -> second (While cs:) <$> parse' bs i
      _                -> abort $ "unmatched [ at byte " ++ show (i + 1)
    parse' (']':bs) i = return (bs, Just (i + 1), [])
    parse' (_:bs)   i = parse' bs (i + 1)

type State = ([Word8], Word8, [Word8])

execute :: [BF] -> IO ()
execute codes = void $ execute' codes ([], 0, [])
  where
    execute' :: [BF] -> State -> IO State
    execute' [] s                     = return s
    execute' (Incr:bs) (xs, x, ys)    = execute' bs (xs, x + 1, ys)
    execute' (Decr:bs) (xs, x, ys)    = execute' bs (xs, x - 1, ys)
    execute' (Next:bs) (xs, x, [])    = execute' bs (x:xs, 0, [])
    execute' (Next:bs) (xs, x, y:ys)  = execute' bs (x:xs, y, ys)
    execute' (Prev:bs) (x:xs, y, ys)  = execute' bs (xs, x, y:ys)
    execute' (Prev:bs) ([], _, _)     = abort "negative address access"
    execute' (While _:bs) s@(_, 0, _) = execute' bs s
    execute' cs@(While bs:_) s        = execute' bs s >>= execute' cs
    execute' (Putc:bs) s@(_, x, _)    =
      (putChar (chr (fromIntegral x)) >> execute' bs s)
        `catchIOError` \e -> abort (show e)
    execute' (Getc:bs) s@(xs, _, ys)  =
      (getChar >>= \x -> execute' bs (xs, fromIntegral (ord x), ys))
        `catchIOError` \case e | isEOFError e -> exitSuccess
                               | otherwise    -> abort (show e)

main :: IO ()
main = do
  hSetBinaryMode stdin True
  hSetBinaryMode stdout True
  hSetBuffering stdout NoBuffering
  getArgs
    >>= \case [] -> getContents; file:_ -> readFile file
    >>= parse
    >>= execute
