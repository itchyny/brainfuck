-- Brainfuck interpreter in Haskell
import Data.Char (chr, ord)
import Control.Monad (void)
import Control.Arrow (first, app, (***), (>>>))

data BF = Incr | Decr | Next | Prev | Put | Get | While [BF]
type State = ([Int], Int, [Int])

(<>>) :: a -> ([a], b) -> ([a], b)
(<>>) = first . (:)

parse :: String -> [BF]
parse = fst . parse'
  where
    parse' :: String -> ([BF], String)
    parse' ('+':bs) = Incr <>> parse' bs
    parse' ('-':bs) = Decr <>> parse' bs
    parse' ('>':bs) = Next <>> parse' bs
    parse' ('<':bs) = Prev <>> parse' bs
    parse' ('.':bs) = Put <>> parse' bs
    parse' (',':bs) = Get <>> parse' bs
    parse' ('[':bs) = app $ ((<>>) . While *** parse') (parse' bs)
    parse' (']':bs) = ([], bs)
    parse' (_:bs)   = parse' bs
    parse' []       = ([], [])

run :: [BF] -> IO ()
run = void . flip run' ([], 0, [])
  where
    run' :: [BF] -> State -> IO State
    run' (Incr:bs) (xs, x, ys)    = run' bs (xs, x + 1, ys)
    run' (Decr:bs) (xs, x, ys)    = run' bs (xs, x - 1, ys)
    run' (Next:bs) (xs, x, [])    = run' bs (x:xs, 0, [])
    run' (Next:bs) (xs, x, y:ys)  = run' bs (x:xs, y, ys)
    run' (Prev:bs) s@([] , _, _)  = run' bs s
    run' (Prev:bs) (x:xs, y, ys)  = run' bs (xs, x, y:ys)
    run' (While _:bs) s@(_, 0, _) = run' bs s
    run' bbs@(While bs:_) s       = run' bs s >>= run' bbs
    run' (Put:bs) s@(_, x, _)     = putChar (chr x) >> run' bs s
    run' (Get:bs) (xs, _, ys)     = getChar >>= \x -> run' bs (xs, ord x, ys)
    run' [] s                     = return s

main :: IO ()
main = readFile "hello.bf" >>= (parse >>> run)

