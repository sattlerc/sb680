{-# LANGUAGE TupleSections #-}
{- Input:  List of points emitted by the calibration tool.
           Touches must have been touched in order of reference_point below.
           Parameter d must match.

  Output:  Transformation matrix from normalized whiteboard coordinate system
           to normalized screen coordinate system.
-}
module Main where

import Control.Monad
import Data.List
import Numeric.LinearAlgebra
import Text.Printf
import System.IO

type Point = Vector R
type System = (Matrix R, Vector R)
type Solution = Vector R

d :: R
d = 0.1

reference_points :: [Point]
reference_points = from_tuples [(d, d), (1 - d, d), (d, 1 - d), (1 - d, 1 - d)]

parse_points :: IO [Point]
parse_points = do
  s <- getContents
  let ls = filter (\v -> size v /= 0) $ map (fromList . map read .words) $ lines s
  forM_ ls (\v -> guard $ size v == 2)
  return ls

partition_points :: [Point] -> [[Point]]
partition_points ps = h ps
  where
    scale :: R
    scale = maximum [norm_2 (b - a) | a <- ps, b <- ps]
    
    h :: [Point] -> [[Point]]
    h [] = []
    h xs@(a : _) = let
      (ys, zs) = partition (\b -> norm_2 (b - a) < scale / 8) xs in
      ys : h zs
    
from_tuples :: [(R, R)] -> [Point]
from_tuples = map $ \(x, y) -> fromList [x, y]

to_scale :: R -> ([R], R) -> ([R], R)
to_scale p (xs, y) = (map (/ p) xs, y / p)

get_line_of_system :: Solution -> (Point, Point) -> [([R], R)]
get_line_of_system r (s, t) = let
  [sx, sy] = toList s
  [tx, ty] = toList t
  in map (to_scale (subVector 6 2 r <.> s + 1))
     [ ([sx, sy, 1, 0, 0, 0, -tx * sx, -tx * sy], tx)
     , ([0, 0, 0, sx, sy, 1, -ty * sx, -ty * sy], ty)
     ]

get_system :: [(Point, Point)] -> Solution -> System
get_system pts r = let
  a = concat (map (get_line_of_system r) pts)
  m = fromLists $ map fst a
  v = fromList $ map snd a
  in
    (m, v)

initial_solution :: Solution
initial_solution = subVector 0 8 $ flatten $ ident 3

get_solution :: System -> Solution
get_solution (m, v) = m <\> v

solve :: [(Point, Point)] -> Solution -> Solution
solve pts = get_solution . get_system pts

get_error :: [(Point, Point)] -> Solution -> R
get_error pts r = let
  (m, v) = get_system pts r
  in
    norm_2 (m #> r - v)

print_result :: Solution -> IO ()
print_result = mapM_ (l . toList) . toRows . reshape 3 . vjoin . (\r -> [r, fromList [1]])
  where
    l :: [R] -> IO ()
    l [x, y, z] = do
      printf "% 1.6f % 1.6f % 1.6f\n" x y z

main :: IO ()
main = do
  ps <- parse_points
  let pps = partition_points ps
  unless (length pps == length reference_points) $
    fail "Provided points do not partition into exactly one group per reference point!"
  let pts = concat $ zipWith (\r -> map (, r)) reference_points pps
  let r = iterate (solve pts) initial_solution !! 10
  print_result r
  hPutStrLn stderr $ "error: " ++ show (get_error pts r)
  return ()
