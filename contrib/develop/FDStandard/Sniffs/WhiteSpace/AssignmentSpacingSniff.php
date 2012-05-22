<?php
/**
 * FDStandard_Sniffs_WhiteSpace_AssignmentSpacingSniff.
 *
 * Checks that assignment blocks have aligned equals signs
 *
 * Modified version of PHP_CodeSniffer Generic_Sniffs_Function_FunctionCallArgumentSpacingSniff
 * by Greg Sherwood <gsherwood@squiz.net> and Marc McIntyre <mmcintyre@squiz.net>
 *
 * @author    Côme Bernigaud <come.bernigaud@laposte.net>
 * @license   http://matrix.squiz.net/developer/tools/php_cs/licence BSD Licence
 */

class FDStandard_Sniffs_WhiteSpace_AssignmentSpacingSniff implements PHP_CodeSniffer_Sniff
{
    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array
     */
    public function register()
    {
        return PHP_CodeSniffer_Tokens::$assignmentTokens;

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

        //~ $blockClosers = array(T_CLOSE_CURLY_BRACKET, T_CLOSE_SQUARE_BRACKET, T_CLOSE_PARENTHESIS);

        if ($prevAssignment = $phpcsFile->findPrevious(array(T_OPEN_PARENTHESIS),$stackPtr-1)) {
          if ($tokens[$prevAssignment]['line'] == $tokens[$stackPtr]['line']) {
            return; // It's not an assignment block
          }
        }
        if ($prevAssignment = $phpcsFile->findPrevious(PHP_CodeSniffer_Tokens::$assignmentTokens,$stackPtr-1)) {
          if ($tokens[$prevAssignment]['line'] == $tokens[$stackPtr]['line']-1) {
            return; // We're not the first assignment of the block
          }
          if ($tokens[$prevAssignment]['line'] == $tokens[$stackPtr]['line']) {
            return; // It's not an assignment block
          }
        }

        $blockLines     = array($stackPtr);
        $nextAssignment = $stackPtr;
        $lastLine       = $tokens[$stackPtr]['line'];

        $bracketTokens = array(T_OPEN_CURLY_BRACKET, T_CLOSE_CURLY_BRACKET,
                                T_OPEN_PARENTHESIS, T_CLOSE_PARENTHESIS);
        if ($nextBracket = $phpcsFile->findNext($bracketTokens,$stackPtr+1)) {
          $nextBracketLine = $tokens[$nextBracket]['line'];
        }

        // Construct the assignments into an array.
        while (($nextAssignment = $phpcsFile->findNext( PHP_CodeSniffer_Tokens::$assignmentTokens,
                                                        ($nextAssignment + 1))) !== false) {
            if (($tokens[$nextAssignment]['line'] - 1) !== $lastLine) {
                // Not part of the block.
                break;
            }
            if (isset($nextBracketLine) && ($tokens[$nextAssignment]['line'] >= $nextBracketLine)) {
                // Not part of the block.
                break;
            }
            if ($tokens[$nextAssignment]['level'] !== $tokens[$stackPtr]['level']) {
                // Not part of the block.
                break;
            }

            $lastLine       = $tokens[$nextAssignment]['line'];
            $blockLines[]   = $nextAssignment;
        }

        $column = $tokens[$stackPtr]['column'];
        foreach ($blockLines as $blockLine) {
          if ($tokens[$blockLine]['column'] != $column) {
            $error = 'Assignment blocks should have all assignment tokens on the same column';
            $phpcsFile->addError($error, $blockLine, 'AssignmentColumn');
          }
        }

    }//end process()


}//end class

?>
