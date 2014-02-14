import Text.Parsec
import Text.Parsec.String
import Text.Parsec.Char as C
import qualified Control.Monad as M

import qualified Text.Parsec.Token as P

  
varident :: Parser String
varident   = many1 C.lower
constident :: Parser String
constident = many1 C.upper
number :: Parser String
number  = many1 C.digit

intconstant :: Parser String
intconstant =  number
                <|> do  a<-  C.string "\'"
                        b<- C.string "\\"
                        c<- C.anyChar
                        d<- C.string "\'"
                        return (a++b++(c:d))
                <|> do  a<- C.string "\'"
                        b<- C.noneOf "'"
                        c<-C.string "\'"
                        return (a++(b:c))

t :: String -> Parser String
t x = spaces >> C.string x 
int :: Parser String

int = do t "uint"
         x<- number
         return ("h_bits("++ x ++ ",false)")
constint =  do len <- int
               t "="
               val <- intconstant
               return ("N_CONSTANT("++len++"," ++ val ++ ","++val++")")
--               lets restrict constraints to what we can express in hammer
constrainedint = do parser <- int
                    option (return parser) (do t("|")
                                               -- choice [(do 
                                               --           t "["
                                               --           lower <- intconstant
                                               --           t ".." 
                                               --           upper <- intconstant
                                               --           t "]"
                                               --           return ("h_int_range("++parser++","++lower++","++upper++")")),
                                               (do      t "["
                                                        values <- sepBy1 (t ",") intconstant
                                                        t "]"
                                                        return ("h_in(" ++ (show values) ++ ","++ (show(length values))++ ")")) )
--                                                      ] )
    

                 
-- --grammar :: Parser String 
-- -- main = do { res <- parseFromFile grammar "new_grammar.nail"
-- --             ; case res of
-- --               Left err -> print err
-- --               Right xs -> print xs }
