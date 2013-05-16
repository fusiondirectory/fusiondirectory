<?php
/**
 * FDStandard_Sniffs_Foreach_ReferenceSniff.
 *
 * Checks that foreach using references are followed by an unset
 *
 * @author    Côme Bernigaud <come.bernigaud@laposte.net>
 * @license   http://matrix.squiz.net/developer/tools/php_cs/licence BSD Licence
 */

class FDStandard_Sniffs_Foreach_ReferenceSniff implements PHP_CodeSniffer_Sniff
{
    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array
     */
    public function register()
    {
        return array(T_FOREACH);

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

        /* Find the as token of the foreach */
        $as = $phpcsFile->findNext(T_AS, $tokens[$stackPtr]['parenthesis_opener']+1, $tokens[$stackPtr]['parenthesis_closer']);
        if (!$as) {
          return;
        }
        /* Search for a reference between as and closing parenthesis  */
        $ref = $phpcsFile->findNext(T_BITWISE_AND, $as+1, $tokens[$stackPtr]['parenthesis_closer']);
        if (!$ref) {
          return;
        }

        $error = 'Foreach token using a reference should unset it afterwards';
        if ($next = $phpcsFile->findNext(T_WHITESPACE, $tokens[$stackPtr]['scope_closer']+1, NULL, TRUE)) {
          if ($tokens[$next]['content'] == 'unset') {
            return;
          }
          $error .= ' (Expected unset, found '.$tokens[$next]['content'].')';
        }

        $phpcsFile->addError($error, $stackPtr, 'UnsetReference');

    }//end process()


}//end class

?>
