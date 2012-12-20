<?php
/**
 * FDStandard_Sniffs_Returns_ParenthesisSniff.
 *
 * Checks that returns does not use parenthesis (expect for logical algebra)
 *
 * @author    Côme Bernigaud <come.bernigaud@laposte.net>
 * @license   http://matrix.squiz.net/developer/tools/php_cs/licence BSD Licence
 */

class FDStandard_Sniffs_Returns_ParenthesisSniff implements PHP_CodeSniffer_Sniff
{
    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array
     */
    public function register()
    {
        return array(T_RETURN);

    }//end register()


    /**
     * Processes this test, when one of its tokens is encountered.
     *
     * @param PHP_CodeSniffer_File $phpcsFile The file being scanned.
     * @param int                  $stackPtr  The position of the current token in the
     *                                        stack passed in $tokens.
     *
     * @return void
     */
    public function process(PHP_CodeSniffer_File $phpcsFile, $stackPtr)
    {
        $tokens = $phpcsFile->getTokens();

        //Check that there is a space after the return token
        if ($tokens[$stackPtr+1]['code'] !== T_WHITESPACE) {
            $error = 'Token "'.$tokens[$stackPtr]['content'].'" should be followed by a space';
            $phpcsFile->addError($error, $stackPtr, 'ReturnAfterSpace');
        }

        $nonWhiteSpace = $stackPtr+1;
        while (in_array($tokens[$nonWhiteSpace]['code'], PHP_CodeSniffer_Tokens::$emptyTokens))
        {
            $nonWhiteSpace++;
        }

        if ($tokens[$nonWhiteSpace]['code'] === T_OPEN_PARENTHESIS) {
            /* Check that this parenthesis block englobe the whole return content */
            $openingParenthesis = $tokens[$nonWhiteSpace]['parenthesis_opener'];
            $closingParenthesis = $tokens[$nonWhiteSpace]['parenthesis_closer'];
            $nonWhiteSpace = $closingParenthesis+1;
            while (in_array($tokens[$nonWhiteSpace]['code'], PHP_CodeSniffer_Tokens::$emptyTokens))
            {
                $nonWhiteSpace++;
            }
            $semiColon = $phpcsFile->findNext(T_SEMICOLON,$stackPtr+1);
            if ($semiColon == $nonWhiteSpace) {
                /* Check that this is not logical algebra */
                $tobecheckedToken = $openingParenthesis;
                $parenthesisAuthorized    = PHP_CodeSniffer_Tokens::$comparisonTokens;
                $parenthesisAuthorized[]  = T_INLINE_THEN;
                $ok = FALSE;
                while ($tobecheckedToken = $phpcsFile->findNext($parenthesisAuthorized, $tobecheckedToken+1)) {
                    if ($tobecheckedToken > $closingParenthesis) {
                        break;
                    }
                    if (end($tokens[$tobecheckedToken]['nested_parenthesis']) == $closingParenthesis) {
                        $ok = TRUE;
                        break;
                    }
                }
                if (!$ok) {
                    /* Complain */
                    $error = 'Token "'.$tokens[$stackPtr]['content'].'" should not use parenthesis';
                    $phpcsFile->addError($error, $stackPtr, 'ReturnParenthesis');
                }
            }
        }

    }//end process()


}//end class

?>
